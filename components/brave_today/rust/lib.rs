
extern crate feed_rs;
use feed_rs::parser;

#[cxx::bridge(namespace = brave_news)] mod ffi {
  pub struct FeedItem {
    id: String,
    title: String,
    description: String,
    image_url: String,
    destionation_url: String,
    published_timestamp: i64,
  }

  pub struct FeedData {
    id: String,
    title: String,
    items: Vec<FeedItem>
  }

  extern "Rust" {
    fn parse_feed_string(source: String, output: &mut FeedData) -> bool;
  }
}

fn parse_feed_string(source: String, output: &mut ffi::FeedData) -> bool {
  let feed_result = parser::parse(source.as_bytes());
  if feed_result.is_err() {
    return false;
  }
  let feed = feed_result.unwrap();
  output.title = if feed.title.is_some() { feed.title.unwrap().content } else { String::from("") };
  for feed_item_data in feed.entries {
    if feed_item_data.links.len() == 0 || feed_item_data.published.is_none() || (feed_item_data.title.is_none() && feed_item_data.summary.is_none()) || feed_item_data.published.is_none() {
      continue;
    }
    let mut image_url: String = String::from("");
    if feed_item_data.media.len() > 0 {
      for media_object in feed_item_data.media {
        if media_object.content.len() > 0 {
          let mut largest_width = 0;
          for content_item in media_object.content {
            if content_item.url.is_some() {
              let this_width = content_item.width.unwrap_or(0);
              if this_width >= largest_width {
                image_url = String::from(content_item.url.unwrap().as_str());
                largest_width = this_width;
              }
            }
          }
        }
        if image_url.is_empty() == false {
          continue;
        }
        if media_object.thumbnails.len() > 0 {
          let mut largest_width = 0;
          for content_item in media_object.thumbnails {
            if content_item.image.uri.is_empty() == false {
              let this_width = content_item.image.width.unwrap_or(0);
              if this_width >= largest_width {
                image_url = content_item.image.uri;
                largest_width = this_width;
              }
            }
          }
        }
      }
    }

    let mut summary: String = String::from("");
    if feed_item_data.summary.is_some() {
      summary = feed_item_data.summary.unwrap().content;
    } else if feed_item_data.content.is_some() {
      summary = feed_item_data.content.unwrap().body.unwrap_or(String::from(""));
    }
    let feed_item = ffi::FeedItem {
      id: feed_item_data.id,
      title: if feed_item_data.title.is_some() { feed_item_data.title.unwrap().content } else { summary.clone() },
      description: summary.clone(),
      image_url: image_url,
      destionation_url: feed_item_data.links[0].href.clone(),
      published_timestamp: feed_item_data.published.unwrap().timestamp()
    };
    output.items.push(feed_item);
  }
  return true;
}
