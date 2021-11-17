// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef BRAVE_COMPONENTS_BRAVE_TODAY_BROWSER_DIRECT_FEED_CONTROLLER_H_
#define BRAVE_COMPONENTS_BRAVE_TODAY_BROWSER_DIRECT_FEED_CONTROLLER_H_

#include <vector>
#include "base/callback_forward.h"
#include "base/containers/fixed_flat_set.h"
#include "brave/components/api_request_helper/api_request_helper.h"
#include "brave/components/brave_today/common/brave_news.mojom-forward.h"

namespace brave_news {

constexpr auto kTemporaryHardcodedFeedUrls = base::MakeFixedFlatSet<base::StringPiece>({
    "http://cnnespanol.cnn.com/feed/",
    "http://newsrss.bbc.co.uk/rss/spanish/news/rss.xml",
    "http://www.elpais.com/rss/feed.html?feedId=17046",
    "https://feeds.feedburner.com/RockPaperShotgun",
    "https://www.wired.co.uk/rss",
    "https://css-tricks.com/feed/",
    "https://feeds.feedburner.com/HighScalability",
    "https://blog.producthunt.com/feed",
    "https://www.nba.com/warriors/rss.xml",
    "https://www.popularwoodworking.com/feed/",
    "https://www.reddit.com/r/brave_browser/.rss",
    "https://www.theguardian.com/football/arsenal/rss",
    "https://www.realclearpolitics.com/rss",
    "https://www.france24.com/en/rss",
    "https://www.haaretz.com/cmlink/1.4605102",
    "https://planet.mozilla.org/rss20.xml"});

using Articles = std::vector<mojom::ArticlePtr>;
using GetArticlesCallback = base::OnceCallback<void(Articles)>;
using GetFeedItemsCallback =
    base::OnceCallback<void(std::vector<mojom::FeedItemPtr>)>;

// Controls RSS / Atom / JSON / etc. feeds - those downloaded
// directly from the feed source server.
class DirectFeedController {
 public:
  explicit DirectFeedController(
      api_request_helper::APIRequestHelper* api_request_helper);
  ~DirectFeedController();
  DirectFeedController(const DirectFeedController&) = delete;
  DirectFeedController& operator=(const DirectFeedController&) = delete;

  void VerifyFeedUrl(const GURL& feed_url,
      base::OnceCallback<void(bool)> callback);
  void DownloadAllContent(GetFeedItemsCallback callback);

 private:
  void DownloadFeedContent(const GURL& feed_url, GetArticlesCallback callback);
  api_request_helper::APIRequestHelper* api_request_helper_;
};

}  // namespace brave_news

#endif  // BRAVE_COMPONENTS_BRAVE_TODAY_BROWSER_DIRECT_FEED_CONTROLLER_H_