// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "asset_manager.h"

#include "nlohmann/json.hpp"

#include "comet/core/conf/configuration_manager.h"
#include "comet/core/file_system.h"
#include "comet/core/generator.h"
#include "comet/resource/resource.h"
#include "comet/resource/resource_manager.h"
#include "editor/asset/asset_utils.h"
#include "editor/asset/exporter/model/model_exporter.h"
#include "editor/asset/exporter/shader/shader_exporter.h"
#include "editor/asset/exporter/shader/shader_module_exporter.h"
#include "editor/asset/exporter/texture/texture_exporter.h"

namespace comet {
namespace editor {
namespace asset {
AssetManager& AssetManager::Get() {
  static AssetManager singleton{};
  return singleton;
}

AssetManager::AssetManager() : Manager{} {
  root_asset_path_ = GetCurrentDirectory();
  root_asset_path_ /= COMET_TCHAR("assets");
  COMET_DISALLOW_STR_ALLOC(root_asset_path_);
  Clean(root_asset_path_);

  constexpr auto library_suffix_len{
      8 + kCometEditorAssetMetadataFileExtension.GetLength()};

  // Add 1 for the separating slash, if needed.
  library_meta_path_.Reserve(
      root_asset_path_.GetLength() + 8 +
      kCometEditorAssetMetadataFileExtension.GetLength() + 1);
  COMET_DISALLOW_STR_ALLOC(library_meta_path_);
  library_meta_path_ = root_asset_path_;
  library_meta_path_ /= COMET_TCHAR("library.");
  library_meta_path_ += kCometEditorAssetMetadataFileExtension;
  Clean(library_meta_path_);
}

void AssetManager::Initialize() {
  Manager::Initialize();
  RefreshLibraryMetadataFile();

  root_resource_path_ = resource::ResourceManager::Get().GetRootResourcePath();
  COMET_DISALLOW_STR_ALLOC(root_resource_path_);

  exporters_.push_back(std::make_unique<ModelExporter>());
  exporters_.push_back(std::make_unique<ShaderExporter>());
  exporters_.push_back(std::make_unique<ShaderModuleExporter>());
  exporters_.push_back(std::make_unique<TextureExporter>());

  for (const auto& exporter : exporters_) {
    exporter->SetRootResourcePath(root_resource_path_);
    exporter->SetRootAssetPath(root_asset_path_);
  }

  Refresh();
}

void AssetManager::RefreshLibraryMetadataFile() {
  SaveMetadata(library_meta_path_, SetAndGetMetadata(library_meta_path_));
}

void AssetManager::Shutdown() {
  is_force_refresh_ = false;
  last_update_time_ = 0;
  root_asset_path_.Clear();
  root_resource_path_.Clear();
  library_meta_path_.Clear();
  exporters_.clear();
  Manager::Shutdown();
}

void AssetManager::Refresh() { Refresh(root_asset_path_); }

void AssetManager::Refresh(CTStringView asset_abs_path) {
  if (IsDirectory(asset_abs_path)) {
    RefreshFolder(asset_abs_path);
  } else if (IsFile(asset_abs_path)) {
    RefreshAsset(asset_abs_path);
  } else {
    COMET_LOG_GLOBAL_ERROR("Bad path given: ", asset_abs_path);
    return;
  }

  RefreshLibraryMetadataFile();
}

const TString& AssetManager::GetAssetsRootPath() const noexcept {
  return root_asset_path_;
}

const TString& AssetManager::GetResourcesRootPath() const noexcept {
  return root_resource_path_;
}

void AssetManager::RefreshFolder(CTStringView asset_abs_path) {
  auto parent_path{GetParentPath(asset_abs_path)};
  auto folder_name{GetName(asset_abs_path)};

  if (folder_name.IsEmpty()) {
    COMET_LOG_GLOBAL_ERROR(
        "Empty folder name retrieved from path: ", asset_abs_path, "!");
    return;
  }

  // Add 1 for separating slash (if necessary).
  TString metadata_file_path{};
  metadata_file_path.Reserve(
      parent_path.GetLength() + folder_name.GetLength() + 1 +
      kCometEditorAssetFolderMetadataFileExtension.GetLength());
  COMET_DISALLOW_STR_ALLOC(metadata_file_path);
  metadata_file_path = parent_path;
  metadata_file_path /= folder_name;  // No allocation.
  metadata_file_path +=
      kCometEditorAssetFolderMetadataFileExtension;  // No allocation.

  if (asset_abs_path != root_asset_path_) {
    SetAndGetMetadata(metadata_file_path);
  }

  ForEachDirectory(asset_abs_path, [&](CTStringView directory_path) {
    RefreshFolder(directory_path);
  });

  ForEachFile(asset_abs_path,
              [&](CTStringView file_path) { RefreshAsset(file_path); });
}

void AssetManager::RefreshAsset(CTStringView asset_abs_path) {
  auto asset_metadata_file_path{GenerateAssetMetadataFilePath(asset_abs_path)};

  if (!IsRefreshNeeded(asset_abs_path, asset_metadata_file_path)) {
    return;
  }

  if (IsMetadataFile(asset_abs_path)) {
    return;
  }

  for (const auto& exporter : exporters_) {
    if (exporter->IsCompatible(GetExtension(asset_abs_path))) {
      exporter->Process(asset_abs_path);
    }
  }
}

bool AssetManager::IsRefreshNeeded(CTStringView asset_abs_path,
                                   CTStringView metadata_file_path) const {
  if (is_force_refresh_ || asset_abs_path == root_asset_path_ ||
      !Exists(metadata_file_path)) {
    return true;
  }

  const auto resource_id{resource::GenerateResourceIdFromPath(
      GetRelativePath(asset_abs_path, root_asset_path_))};

  if (!Exists(GenerateResourcePath(root_resource_path_, resource_id))) {
    return true;
  }

  // We must use assignment here to prevent a bug with GCC where the generated
  // type is an array (which is wrong).
  const auto existing_metadata = GetMetadata(metadata_file_path);

  const auto update_time{existing_metadata.value(
      kCometEditorAssetMetadataKeyUpdateTime, static_cast<f64>(-1))};

  const auto modification_time{GetLastModificationTime(asset_abs_path)};

  return update_time <= modification_time;
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
