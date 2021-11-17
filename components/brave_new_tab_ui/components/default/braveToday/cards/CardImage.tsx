// Copyright (c) 2020 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import * as React from 'react'
import * as Card from '../cardSizes'
import getBraveNewsController, * as BraveNews from '../../../../api/brave_news'
import { getDataUrl } from '../../../../../common/privateCDN'

type Props = {
  imageUrl: string
  list?: boolean
  isUnpadded?: boolean
  isPromoted?: boolean
  onLoaded?: () => any
}

function useGetUnpaddedImage (paddedUrl: string, isUnpadded: boolean, onLoaded?: () => any) {
  const [unpaddedUrl, setUnpaddedUrl] = React.useState('')
  const onReceiveUnpaddedUrl = (result: string) => {
    setUnpaddedUrl(result)
    window.requestAnimationFrame(() => {
      if (onLoaded) {
        onLoaded()
      }
    })
  }
  React.useEffect(() => {
    // Storybook method
    // @ts-expect-error
    if (window.braveStorybookUnpadUrl) {
      // @ts-expect-error
      window.braveStorybookUnpadUrl(paddedUrl)
      .then(onReceiveUnpaddedUrl)
      return
    }

    getBraveNewsController().getImageData({ url: paddedUrl })
    .then(async (result) => {
      if (!result.imageData) {
        return
      }
      const resultBuffer = new Uint8Array(result.imageData).buffer
      const dataUrl = await getDataUrl(resultBuffer)
      onReceiveUnpaddedUrl(dataUrl)
    })
    .catch(err => {
      console.error(`Error getting image for ${paddedUrl}.`, err)
    })
  }, [paddedUrl, isUnpadded])
  return unpaddedUrl
}

export default function CardImage (props: Props) {
  const unpaddedUrl = useGetUnpaddedImage(props.imageUrl, !!props.isUnpadded, props.onLoaded)
  const [isImageLoaded, setIsImageLoaded] = React.useState(false)
  React.useEffect(() => {
    if (unpaddedUrl) {
      const img = new Image()
      let shouldCancel = false
      img.addEventListener('load', () => {
        if (!shouldCancel) {
          setIsImageLoaded(true)
        }
      })
      img.src = unpaddedUrl
      return () => { shouldCancel = true }
    }

    return () => { }
  }, [unpaddedUrl])
  const Frame = props.list ? Card.ListImageFrame : Card.ImageFrame
  return (
    <Frame data-source={props.imageUrl} isImageLoaded={isImageLoaded}>
      <Card.Image isPromoted={props.isPromoted} src={unpaddedUrl} />
    </Frame>
  )
}

type FromFeedItemProps = Omit<Props, 'imageUrl' | 'isUnpadded'> & {
  data: BraveNews.FeedItemMetadata
}

const imgSourceRegex = /<img[^>]+src="([^">]+)"/gmi

export function CardImageFromFeedItem (props: FromFeedItemProps) {
  React.useEffect(() => {
    if (!props.data.image.imageUrl && !props.data.image.paddedImageUrl) {
      // Shouldn't happen since backend filters out items
      // with no image. This is in a useEffect so it does not log every render.
      console.error('Brave News found item with no image', props.data.url.url)
    }
  }, [props.data.image.imageUrl, props.data.image.paddedImageUrl])
  const imageUrl = React.useMemo(() => {
    if (props.data.image.imageUrl?.url) { return props.data.image.imageUrl.url }
    if (props.data.image.paddedImageUrl?.url) { return props.data.image.paddedImageUrl.url }
    // get first image source match
    const content = props.data.description
    if (!content) { return '' }
    const matches = [...content.matchAll(imgSourceRegex)]
    if (matches.length) {
      // Get first match, last one is often a tracking pixel
      return matches[0][1]
    }
    return ''
  }, [props.data.image.imageUrl, props.data.image.paddedImageUrl, props.data.description])
  const { data, ...baseProps } = props
  if (imageUrl) {
    return (
      <CardImage
        {...baseProps}
        imageUrl={imageUrl}
        isPromoted={props.isPromoted}
      />
    )
  }
  return null
}
