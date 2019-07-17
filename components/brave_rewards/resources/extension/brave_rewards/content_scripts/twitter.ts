/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

// Utils
import { getMessage } from '../background/api/locale_api'

let timeout: any = null

const getTwitterAPICredentials = () => {
  const msg = { type: 'getTwitterAPICredentials' }
  return new Promise(resolve => chrome.runtime.sendMessage(msg, resolve))
}

const getTweetDetails = async (tweetId: string) => {
  const credentialHeaders = await getTwitterAPICredentials()
  const url = new URL('https://api.twitter.com/1.1/statuses/show.json')
  url.searchParams.append('id', tweetId)
  const response = await fetch(url.toString(), {
    credentials: 'include',
    headers: {
      ...credentialHeaders
    },
    referrerPolicy: 'no-referrer-when-downgrade',
    method: 'GET',
    mode: 'cors',
    redirect: 'follow'
  })
  return response.json()
}

const getTweetMetaData = (tweet: Element): Promise<RewardsTip.MediaMetaData> => {
  if (!tweet) {
    return Promise.reject(null)
  }

  const status = tweet.querySelector("a[href*='/status/']") as HTMLAnchorElement
  if (!status || !status.href) {
    return Promise.reject(null)
  }

  const tweetIdMatches = status.href.match(/status\/(\d+)/)
  if (!tweetIdMatches || tweetIdMatches.length < 2) {
    return Promise.reject(null)
  }

  const tweetId = tweetIdMatches[1]

  return getTweetDetails(tweetId)
    .then(tweetDetails => {
      const mediaMetadata: RewardsTip.MediaMetaData = {
        mediaType: 'twitter',
        twitterName: tweetDetails.user.name,
        screenName: tweetDetails.user.screen_name,
        userId: tweetDetails.user.id_str,
        tweetId: tweetId,
        tweetTimestamp: Date.parse(tweetDetails.created_at) / 1000,
        tweetText: tweetDetails.text
      }
      return mediaMetadata
    })
    .catch(error => {
      console.log(`Failed to fetch tweet details for ${tweetId}: ${error.message}`)
      return Promise.reject(error)
    })
}

const getTweetMetaDataForOldTwitter = (tweet: Element): RewardsTip.MediaMetaData | null => {
  if (!tweet) {
    return null
  }

  const tweetTextElements = tweet.getElementsByClassName('tweet-text')
  if (!tweetTextElements || tweetTextElements.length === 0) {
    return null
  }

  const tweetText = tweetTextElements[0] as HTMLElement

  const tweetTimestampElements = tweet.getElementsByClassName('js-short-timestamp')
  if (!tweetTimestampElements || tweetTimestampElements.length === 0) {
    return null
  }

  const tweetTimestamp = tweetTimestampElements[0].getAttribute('data-time') || ''

  return {
    mediaType: 'twitter',
    twitterName: tweet.getAttribute('data-name') || '',
    screenName: tweet.getAttribute('data-screen-name') || '',
    userId: tweet.getAttribute('data-user-id') || '',
    tweetId: tweet.getAttribute('data-tweet-id') || '',
    tweetTimestamp: parseInt(tweetTimestamp, 10) || 0,
    tweetText: tweetText.innerText || ''
  }
}

const tipTwitterUser = (mediaMetaData: RewardsTip.MediaMetaData) => {
  const msg = { type: 'tipInlineMedia', mediaMetaData }
  chrome.runtime.sendMessage(msg)
}

const createBraveTipAction = (tweet: Element, newTwitter: boolean) => {
  // Create the tip action
  const braveTipAction = document.createElement('div')
  braveTipAction.className = 'ProfileTweet-action js-tooltip action-brave-tip'
  braveTipAction.style.display = 'inline-block'
  braveTipAction.style.minWidth = '80px'
  braveTipAction.setAttribute('data-original-title', getMessage('twitterTipsHoverText'))

  // Create the tip button
  const braveTipButton = document.createElement('button')
  braveTipButton.className = 'ProfileTweet-actionButton u-textUserColorHover js-actionButton'
  braveTipButton.style.background = 'transparent'
  braveTipButton.style.border = '0'
  braveTipButton.style.color = '#657786'
  braveTipButton.style.cursor = 'pointer'
  braveTipButton.style.display = 'inline-block'
  braveTipButton.style.fontSize = '16px'
  braveTipButton.style.lineHeight = '1'
  braveTipButton.style.outline = '0'
  braveTipButton.style.padding = '0 2px'
  braveTipButton.style.position = 'relative'
  braveTipButton.type = 'button'
  braveTipButton.onclick = function (event) {
    if (newTwitter) {
      getTweetMetaData(tweet)
        .then(tweetMetaData => {
          if (tweetMetaData) {
            tipTwitterUser(tweetMetaData)
          }
        })
        .catch(error => {
          console.error(`Failed to fetch tweet metadata for ${tweet}:`, error)
        })
    } else {
      const tweetMetaData = getTweetMetaDataForOldTwitter(tweet)
      if (tweetMetaData) {
        tipTwitterUser(tweetMetaData)
      }
    }
    event.stopPropagation()
  }

  if (newTwitter && tweet && tweet.getAttribute('data-testid') === 'tweetDetail') {
    braveTipButton.style.marginTop = '12px'
  }

  // Create the tip icon container
  const braveTipIconContainer = document.createElement('div')
  braveTipIconContainer.className = 'IconContainer js-tooltip'
  braveTipIconContainer.style.display = 'inline-block'
  braveTipIconContainer.style.lineHeight = '0'
  braveTipIconContainer.style.position = 'relative'
  braveTipIconContainer.style.verticalAlign = 'middle'
  braveTipButton.appendChild(braveTipIconContainer)

  // Create the tip icon
  const braveTipIcon = document.createElement('span')
  braveTipIcon.className = 'Icon Icon--medium'
  braveTipIcon.style.background = 'transparent'
  braveTipIcon.style.content = 'url(\'data:image/svg+xml;utf8,<svg version="1.1" id="Layer_1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" x="0px" y="0px" viewBox="0 0 105 100" style="enable-background:new 0 0 105 100;" xml:space="preserve"><style type="text/css">.st1{fill:%23662D91;}.st2{fill:%239E1F63;}.st3{fill:%23FF5000;}.st4{fill:%23FFFFFF;stroke:%23FF5000;stroke-width:0.83;stroke-miterlimit:10;}</style><title>BAT_icon</title><g id="Layer_2_1_"><g id="Layer_1-2"><polygon class="st1" points="94.8,82.6 47.4,55.4 0,82.9 "/><polygon class="st2" points="47.4,0 47.1,55.4 94.8,82.6 "/><polygon class="st3" points="0,82.9 47.2,55.9 47.4,0 "/><polygon class="st4" points="47.1,33.7 28,66.5 66.7,66.5 "/></g></g></svg>\')'
  braveTipIcon.style.display = 'inline-block'
  braveTipIcon.style.fontSize = '18px'
  braveTipIcon.style.fontStyle = 'normal'
  braveTipIcon.style.height = '16px'
  braveTipIcon.style.marginTop = '5px'
  braveTipIcon.style.position = 'relative'
  braveTipIcon.style.verticalAlign = 'baseline'
  braveTipIcon.style.width = '16px'
  braveTipIconContainer.appendChild(braveTipIcon)

  // Create the tip action count (typically used to present a counter
  // associated with the action, but we'll use it to display a static
  // action label)
  const braveTipActionCount = document.createElement('span')
  braveTipActionCount.className = 'ProfileTweet-actionCount'
  braveTipActionCount.style.color = '#657786'
  braveTipActionCount.style.display = 'inline-block'
  braveTipActionCount.style.fontSize = '12px'
  braveTipActionCount.style.fontWeight = 'bold'
  braveTipActionCount.style.lineHeight = '1'
  braveTipActionCount.style.marginLeft = '6px'
  braveTipActionCount.style.position = 'relative'
  braveTipActionCount.style.verticalAlign = 'text-bottom'
  braveTipButton.appendChild(braveTipActionCount)

  // Create the tip action count presentation
  const braveTipActionCountPresentation = document.createElement('span')
  braveTipActionCountPresentation.className = 'ProfileTweet-actionCountForPresentation'
  braveTipActionCountPresentation.textContent = getMessage('twitterTipsIconLabel')
  braveTipActionCount.appendChild(braveTipActionCountPresentation)

  // Create the shadow DOM root that hosts our injected DOM elements
  const shadowRoot = braveTipAction.attachShadow({ mode: 'open' })
  shadowRoot.appendChild(braveTipButton)

  // Create style element for hover color
  const style = document.createElement('style')
  style.innerHTML = '.ProfileTweet-actionButton :hover { color: #FB542B }'
  shadowRoot.appendChild(style)

  return braveTipAction
}

const configureBraveTipAction = () => {
  clearTimeout(timeout)
  chrome.runtime.sendMessage('rewardsEnabled', function (rewards) {
    const msg = {
      type: 'inlineTipSetting',
      key: 'twitter'
    }
    chrome.runtime.sendMessage(msg, function (inlineTip) {
      const tippingEnabled = rewards.enabled && inlineTip.enabled
      let newTwitter = true
      let tweets = document.querySelectorAll('[data-testid="tweet"], [data-testid="tweetDetail"]')
      if (tweets.length === 0) {
        tweets = document.querySelectorAll('.tweet')
        newTwitter = false
      }
      for (let i = 0; i < tweets.length; ++i) {
        let actions
        if (newTwitter) {
          actions = tweets[i].querySelector('[role="group"]')
        } else {
          actions = tweets[i].querySelector('.js-actions')
        }
        if (actions) {
          const braveTipActions = actions.getElementsByClassName('action-brave-tip')
          if (tippingEnabled && braveTipActions.length === 0) {
            actions.appendChild(createBraveTipAction(tweets[i], newTwitter))
          } else if (!tippingEnabled && braveTipActions.length === 1) {
            actions.removeChild(braveTipActions[0])
          }
        }
      }
    })
  })
  timeout = setTimeout(configureBraveTipAction, 3000)
}

// In order to deal with infinite scrolling and overlays, periodically
// check if injection needs to occur (mitigate the performance cost
// by only running this when the foreground tab is active or visible)
document.addEventListener('visibilitychange', function () {
  clearTimeout(timeout)
  if (!document.hidden) {
    timeout = setTimeout(configureBraveTipAction, 3000)
  }
})

configureBraveTipAction()
