// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef BRAVE_BROWSER_SKUS_SDK_PROVIDER_IMPL_H_
#define BRAVE_BROWSER_SKUS_SDK_PROVIDER_IMPL_H_

#include <string>

#include "brave/components/skus/common/skus_sdk.mojom.h"

namespace skus {

class SdkController;

class SdkProviderImpl final : public skus::mojom::SdkController {
 public:
  SdkProviderImpl(const SdkProviderImpl&) = delete;
  SdkProviderImpl& operator=(const SdkProviderImpl&) = delete;

  explicit SdkProviderImpl(SdkController* service);
  ~SdkProviderImpl() override;

  void RefreshOrder(const std::string& order_id,
                    RefreshOrderCallback callback) override;
  void FetchOrderCredentials(const std::string& order_id,
                             FetchOrderCredentialsCallback callback) override;
  void PrepareCredentialsPresentation(
      const std::string& domain,
      const std::string& path,
      PrepareCredentialsPresentationCallback callback) override;
  void CredentialSummary(const std::string& domain,
                         CredentialSummaryCallback callback) override;

 private:
  SdkController* service_;
};

}  // namespace skus

#endif  // BRAVE_BROWSER_SKUS_SDK_PROVIDER_IMPL_H_
