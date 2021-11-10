/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_AD_BLOCK_ENGINE_SERVICE_H_
#define BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_AD_BLOCK_ENGINE_SERVICE_H_

#include <stdint.h>

#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "base/files/file_path.h"
#include "base/memory/weak_ptr.h"
#include "base/sequence_checker.h"
#include "base/values.h"
#include "brave/components/brave_component_updater/browser/dat_file_util.h"
#include "brave/components/brave_shields/browser/ad_block_resource_provider.h"
#include "brave/components/brave_shields/browser/ad_block_source_provider.h"
#include "brave/components/brave_shields/browser/base_brave_shields_service.h"
#include "third_party/blink/public/mojom/loader/resource_load_info.mojom-shared.h"

using brave_component_updater::BraveComponent;
using brave_component_updater::DATFileDataBuffer;

namespace adblock {
class Engine;
}

class AdBlockServiceTest;
class BraveAdBlockTPNetworkDelegateHelperTest;

namespace brave_shields {

// Brave Shields service managing an adblock engine.
class AdBlockEngineService : public BaseBraveShieldsService,
                             public ResourceProvider::Observer,
                             public SourceProvider::Observer {
 public:
  using GetDATFileDataResult =
      brave_component_updater::LoadDATFileDataResult<adblock::Engine>;

  explicit AdBlockEngineService(
      scoped_refptr<base::SequencedTaskRunner> task_runner);
  ~AdBlockEngineService() override;

  void ShouldStartRequest(const GURL& url,
                          blink::mojom::ResourceType resource_type,
                          const std::string& tab_host,
                          bool aggressive_blocking,
                          bool* did_match_rule,
                          bool* did_match_exception,
                          bool* did_match_important,
                          std::string* replacement_url) override;
  absl::optional<std::string> GetCspDirectives(
      const GURL& url,
      blink::mojom::ResourceType resource_type,
      const std::string& tab_host);
  void AddResources(const std::string& resources);
  void EnableTag(const std::string& tag, bool enabled);
  bool TagExists(const std::string& tag);

  virtual absl::optional<base::Value> UrlCosmeticResources(
      const std::string& url);
  virtual absl::optional<base::Value> HiddenClassIdSelectors(
      const std::vector<std::string>& classes,
      const std::vector<std::string>& ids,
      const std::vector<std::string>& exceptions);

  void OnNewListSourceAvailable(const DATFileDataBuffer& list_source) override;
  void OnNewDATAvailable(const DATFileDataBuffer& list_source) override;

  void OnNewResourcesAvailable(const std::string& resources_json) override;

  void OnInitialListLoad(bool deserialize, const DATFileDataBuffer& dat_buf);

  bool Init(SourceProvider* source_provider,
            ResourceProvider* resource_provider);

 protected:
  void GetDATFileData(const base::FilePath& dat_file_path,
                      bool deserialize = true,
                      base::OnceClosure callback = base::DoNothing());
  void AddKnownTagsToAdBlockInstance();
  void UpdateAdBlockClient(std::unique_ptr<adblock::Engine> ad_block_client);

  std::unique_ptr<adblock::Engine> ad_block_client_;

 private:
  friend class ::AdBlockServiceTest;
  friend class ::BraveAdBlockTPNetworkDelegateHelperTest;

  bool Init() override;

  void UpdateFiltersOnFileTaskRunner(const DATFileDataBuffer& filters);
  void UpdateDATOnFileTaskRunner(const DATFileDataBuffer& dat_buf);

  void DemandResourceReload();

  void OnGetDATFileData(base::OnceClosure callback,
                        GetDATFileDataResult result);
  void OnPreferenceChanges(const std::string& pref_name);

  // `$redirect-url` parsing is security-sensitive, and should only be used for
  // testing purposes.
  void EnableRedirectUrlParsingForTest() { redirect_url_parsing_ = true; }

  bool redirect_url_parsing_ = false;

  std::set<std::string> tags_;

  ResourceProvider* resource_provider_;

  base::WeakPtrFactory<AdBlockEngineService> weak_factory_{this};

  AdBlockEngineService(const AdBlockEngineService&) = delete;
  AdBlockEngineService& operator=(const AdBlockEngineService&) = delete;
};

// Creates the AdBlockEngineService
std::unique_ptr<AdBlockEngineService> AdBlockEngineServiceFactory(
    scoped_refptr<base::SequencedTaskRunner> task_runner);

}  // namespace brave_shields

#endif  // BRAVE_COMPONENTS_BRAVE_SHIELDS_BROWSER_AD_BLOCK_ENGINE_SERVICE_H_
