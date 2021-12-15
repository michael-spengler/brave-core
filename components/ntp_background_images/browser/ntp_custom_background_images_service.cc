// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/components/ntp_background_images/browser/ntp_custom_background_images_service.h"

#include <utility>

#include "base/notreached.h"
#include "base/values.h"
#include "brave/components/ntp_background_images/browser/url_constants.h"

namespace ntp_background_images {

NTPCustomBackgroundImagesService::NTPCustomBackgroundImagesService(
    std::unique_ptr<Delegate> delegate)
    : delegate_(std::move(delegate)) {
  DCHECK(delegate_);
}

NTPCustomBackgroundImagesService::~NTPCustomBackgroundImagesService() = default;

bool NTPCustomBackgroundImagesService::ShouldShowCustomBackground() const {
  return delegate_->IsCustomBackgroundEnabled();
}

base::Value NTPCustomBackgroundImagesService::GetBackground() const {
  base::Value data(base::Value::Type::DICTIONARY);
  data.SetStringKey(kWallpaperImageURLKey,
                    delegate_->GetCustomBackgroundImageURL());
  data.SetBoolKey(kIsBackgroundKey, true);
  return data;
}

void NTPCustomBackgroundImagesService::Shutdown() {
  NOTIMPLEMENTED();
}

}  // namespace ntp_background_images
