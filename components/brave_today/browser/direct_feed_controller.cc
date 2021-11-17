// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/components/brave_today/browser/direct_feed_controller.h"

#include <codecvt>
#include <iterator>

#include "base/barrier_callback.h"
#include "base/callback.h"

#include "base/logging.h"
#include "base/time/time.h"
#include "brave/components/brave_private_cdn/headers.h"
#include "brave/components/brave_today/common/brave_news.mojom-forward.h"
#include "brave/components/brave_today/common/brave_news.mojom-shared.h"
#include "brave/components/brave_today/common/brave_news.mojom.h"
#include "brave/components/brave_today/rust/lib.rs.h"
#include "ui/base/l10n/time_format.h"

namespace brave_news {

namespace {

  mojom::ArticlePtr RustFeedItemToArticle(FeedItem& rust_feed_item) {
    auto metadata = mojom::FeedItemMetadata::New();
    metadata->title = rust_feed_item.title.c_str();
    metadata->image = mojom::Image::NewImageUrl(
        GURL(rust_feed_item.image_url.c_str()));
    metadata->url = GURL(rust_feed_item.destionation_url.c_str());
    metadata->description = rust_feed_item.description.c_str();
    metadata->publish_time =
        base::Time::FromJsTime(rust_feed_item.published_timestamp * 1000);
    // Get language-specific relative time
    base::TimeDelta relative_time_delta =
        base::Time::Now() - metadata->publish_time;
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    metadata->relative_time_description =
        converter.to_bytes(ui::TimeFormat::Simple(
            ui::TimeFormat::Format::FORMAT_ELAPSED,
            ui::TimeFormat::Length::LENGTH_LONG, relative_time_delta));
    auto article = mojom::Article::New();
    article->data = std::move(metadata);
    // Calculate score same method as brave news aggregator
    auto seconds_since_publish = relative_time_delta.InSeconds();
    article->data->score = std::abs(std::log(seconds_since_publish));
    return article;
  }

}  // namespace

DirectFeedController::DirectFeedController(
    api_request_helper::APIRequestHelper* api_request_helper)
    : api_request_helper_(api_request_helper) { }

DirectFeedController::~DirectFeedController() = default;

void DirectFeedController::VerifyFeedUrl(const GURL& feed_url,
    base::OnceCallback<void(bool)> callback) {
  // Download the feed and once it's done, see if there's any content.
  // This verifies that the URL is reachable, that it has content,
  // and that the content has the correct fields for Brave News.
  DownloadFeedContent(feed_url, base::BindOnce(
    [](base::OnceCallback<void(bool)> callback, Articles results) {
      // Handle response
      bool is_valid = (results.size() > 0);
      std::move(callback).Run(is_valid);
    }, std::move(callback)));
}

void DirectFeedController::DownloadAllContent(GetFeedItemsCallback callback) {
  // Handle when all retrieve operations are complete
  auto all_done_handler = base::BindOnce(
    [](GetFeedItemsCallback callback, std::vector<Articles> results) {
      VLOG(1) << "All direct feeds retrieved.";
      std::size_t total_size = 0;
      for (const auto& collection : results) {
        total_size += collection.size();
      }
      std::vector<mojom::FeedItemPtr> all_feed_articles;
      all_feed_articles.reserve(total_size);
      for (auto& collection : results) {
        auto it = collection.begin();
        while (it != collection.end()) {
          all_feed_articles.insert(all_feed_articles.end(),
              mojom::FeedItem::NewArticle(*std::make_move_iterator(it)));
          it = collection.erase(it);
        }
      }
      std::move(callback).Run(std::move(all_feed_articles));
    }, std::move(callback));
  // Perform requests in parallel and wait for completion
  auto feed_content_handler = base::BarrierCallback<Articles>(
      kTemporaryHardcodedFeedUrls.size(), std::move(all_done_handler));
  for (auto const url : kTemporaryHardcodedFeedUrls) {
    DownloadFeedContent(GURL(url), feed_content_handler);
  }
}

void DirectFeedController::DownloadFeedContent(const GURL& feed_url,
      GetArticlesCallback callback) {
  // Handle Response
  auto responseHandler = base::BindOnce(
    [](GetArticlesCallback callback, const GURL& feed_url, const int status,
         const std::string& body,
         const base::flat_map<std::string, std::string>& headers) {
    // Validate response
    if (status < 200 || status >= 300 || body.empty()) {
      VLOG(1) << feed_url.spec() << " invalid response, status: " << status;
      std::move(callback).Run({});
      return;
    }
    // Reponse is valid, but still might not be a feed
    FeedData data;
    if (!brave_news::parse_feed_string(::rust::String(body), data)) {
      VLOG(1) << feed_url.spec() << " not a valid feed.";
      VLOG(2) << "Response body was:";
      VLOG(2) << body;
      std::move(callback).Run({});
      return;
    }
    // Valid feed, convert items
    VLOG(1) << "Valid feed parsed from " << feed_url.spec();
    Articles articles;
    for (auto entry : data.items) {
      auto item = RustFeedItemToArticle(entry);
      item->data->publisher_id = "direct:" + feed_url.spec();
      articles.emplace_back(std::move(item));
    }
    // Add variety to score, same as brave feed aggregator
    // Sort by score, ascending
    std::sort(articles.begin(), articles.end(), [](mojom::ArticlePtr& a, mojom::ArticlePtr& b) {
      return (a.get()->data->score < b.get()->data->score);
    });
    double variety = 2.0;
    for (auto &entry : articles) {
      entry->data->score = entry->data->score * variety;
      variety = variety * 2.0;
    }
    VLOG(1) << "Direct feed retrieved article count: " << articles.size();
    std::move(callback).Run(std::move(articles));
    }, std::move(callback), feed_url);
  // Make request
  api_request_helper_->Request("GET", feed_url, "", "", true,
                              std::move(responseHandler),
                              brave::private_cdn_headers);
}

}  // namespace brave_news