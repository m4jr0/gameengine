// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_ASSET_MANAGER_H_
#define COMET_EDITOR_ASSET_ASSET_MANAGER_H_

#include "comet_precompile.h"

#include "editor/asset/exporter/asset_exporter.h"

namespace comet {
namespace editor {
namespace asset {
class AssetManager {
 public:
  AssetManager();
  AssetManager(const AssetManager&) = delete;
  AssetManager(AssetManager&&) = delete;
  AssetManager& operator=(const AssetManager&) = delete;
  AssetManager& operator=(AssetManager&&) = delete;
  ~AssetManager() = default;

  void Initialize();
  void RefreshLibraryMetadataFile();
  void Destroy();
  void Refresh();
  void Refresh(const std::string& asset_abs_path);

  const std::string& GetAssetsRootPath() const noexcept;
  const std::string& GetResourcesRootPath() const noexcept;

 private:
  f64 last_update_time_{0};
  std::string root_asset_path_;
  std::string root_resource_path_;
  std::string library_meta_path_;
  bool is_force_refresh_{false};

  std::vector<std::unique_ptr<AssetExporter>> exporters_;

  void RefreshFolder(const std::string& asset_abs_path);
  void RefreshAsset(const std::string& asset_abs_path);
  bool IsRefreshNeeded(const std::string& asset_abs_path,
                       const std::string& meta_file_path);
};
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_ASSET_MANAGER_H_
