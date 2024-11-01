// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_ASSET_MANAGER_H_
#define COMET_EDITOR_ASSET_ASSET_MANAGER_H_

#include "comet/core/concurrency/job/job.h"
#include "comet/core/essentials.h"
#include "comet/core/manager.h"
#include "editor/asset/exporter/asset_exporter.h"

namespace comet {
namespace editor {
namespace asset {
class AssetManager : public Manager {
 public:
  static AssetManager& Get();

  AssetManager();
  AssetManager(const AssetManager&) = delete;
  AssetManager(AssetManager&&) = delete;
  AssetManager& operator=(const AssetManager&) = delete;
  AssetManager& operator=(AssetManager&&) = delete;
  ~AssetManager() = default;

  void Initialize() override;
  void Shutdown() override;
  void RefreshLibraryMetadataFile();
  void Refresh();

  const TString& GetAssetsRootPath() const noexcept;
  const TString& GetResourcesRootPath() const noexcept;

 private:
  static void OnRefresh(job::IOJobParamsHandle params_handle);

  void RefreshLibrary(job::Counter* global_counter);
  void RefreshFolder(job::Counter* global_counter, CTStringView asset_abs_path);
  void RefreshAsset(job::Counter* global_counter, CTStringView asset_abs_path);
  bool IsRefreshNeeded(CTStringView asset_abs_path,
                       CTStringView metadata_file_path) const;

  bool is_force_refresh_{false};
  f64 last_update_time_{0};
  TString root_asset_path_{};
  TString root_resource_path_{};
  TString library_meta_path_{};
  std::vector<std::unique_ptr<AssetExporter>> exporters_{};
};
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_ASSET_MANAGER_H_
