// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "asset_manager.h"

#include "nlohmann/json.hpp"

#include "comet/core/engine.h"
#include "comet/resource/resource.h"
#include "comet/utils/date.h"
#include "comet/utils/file_system.h"
#include "editor/asset/asset_utils.h"
#include "editor/asset/exporter/model_exporter.h"
#include "editor/asset/exporter/shader_exporter.h"
#include "editor/asset/exporter/shader_module_exporter.h"
#include "editor/asset/exporter/texture_exporter.h"

namespace comet {
namespace editor {
namespace asset {
AssetManager::AssetManager()
    : root_asset_path_{utils::filesystem::Append(
          utils::filesystem::GetCurrentDirectory(), "assets")},
      library_meta_path_{utils::filesystem::Append(
          root_asset_path_,
          "library." + std::string(kCometEditorAssetMetadataFileExtension))} {}

void AssetManager::Initialize() {
  Manager::Initialize();
  RefreshLibraryMetadataFile();

  root_resource_path_ =
      Engine::Get().GetResourceManager().GetRootResourcePath();

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
  root_asset_path_.clear();
  root_resource_path_.clear();
  library_meta_path_.clear();
  exporters_.clear();
  Manager::Shutdown();
}

void AssetManager::Refresh() { Refresh(root_asset_path_); }

void AssetManager::Refresh(const schar* asset_abs_path) {
  if (utils::filesystem::IsDirectory(asset_abs_path)) {
    RefreshFolder(asset_abs_path);
  } else if (utils::filesystem::IsFile(asset_abs_path)) {
    RefreshAsset(asset_abs_path);
  } else {
    COMET_LOG_GLOBAL_ERROR("Bad path given: ", asset_abs_path);
    return;
  }

  RefreshLibraryMetadataFile();
}

void AssetManager::Refresh(const std::string& asset_abs_path) {
  Refresh(asset_abs_path.c_str());
}

const std::string& AssetManager::GetAssetsRootPath() const noexcept {
  return root_asset_path_;
}

const std::string& AssetManager::GetResourcesRootPath() const noexcept {
  return root_resource_path_;
}

void AssetManager::RefreshFolder(std::string_view asset_abs_path) {
  const auto folder_name{utils::filesystem::GetNameView(asset_abs_path)};
  std::string metadata_file_path{};
  metadata_file_path.reserve(
      folder_name.size() + kCometEditorAssetFolderMetadataFileExtension.size());
  std::memcpy(metadata_file_path.data(), folder_name.data(),
              folder_name.size());
  std::memcpy(metadata_file_path.data() + folder_name.size(),
              kCometEditorAssetFolderMetadataFileExtension.data(),
              kCometEditorAssetFolderMetadataFileExtension.size());

  metadata_file_path = utils::filesystem::Append(
      utils::filesystem::GetParentPath(asset_abs_path),
      std::move(metadata_file_path));

  if (asset_abs_path != root_asset_path_) {
    SetAndGetMetadata(metadata_file_path);
  }

  const auto folders{utils::filesystem::ListDirectories(asset_abs_path)};

  for (const auto& folder : folders) {
    RefreshFolder(folder);
  }

  const auto assets{utils::filesystem::ListFiles(asset_abs_path)};

  for (const auto& asset : assets) {
    RefreshAsset(asset);
  }
}

void AssetManager::RefreshAsset(const schar* asset_abs_path) {
  if (!IsRefreshNeeded(asset_abs_path,
                       GetAssetMetadataFilePath(asset_abs_path))) {
    return;
  }

  if (IsMetadataFile(asset_abs_path)) {
    return;
  }

  for (const auto& exporter : exporters_) {
    if (exporter->IsCompatible(
            utils::filesystem::GetExtension(asset_abs_path))) {
      exporter->Process(asset_abs_path);
    }
  }
}

void AssetManager::RefreshAsset(const std::string& asset_abs_path) {
  return RefreshAsset(asset_abs_path.c_str());
}

bool AssetManager::IsRefreshNeeded(const schar* asset_abs_path,
                                   const schar* metadata_file_path) const {
  if (is_force_refresh_ || asset_abs_path == root_asset_path_ ||
      !utils::filesystem::Exists(metadata_file_path)) {
    return true;
  }

  const auto asset_path{
      utils::filesystem::GetRelativePath(asset_abs_path, root_asset_path_)};

  const auto resource_id{resource::GenerateResourceIdFromPath(asset_path)};

  if (!utils::filesystem::Exists(utils::filesystem::Append(
          root_resource_path_, std::to_string(resource_id)))) {
    return true;
  }

  // We must use assignment here to prevent a bug with GCC where the generated
  // type is an array (which is wrong).
  const auto existing_metadata = GetMetadata(metadata_file_path);

  const auto update_time{existing_metadata.value(
      kCometEditorAssetMetadataKeyUpdateTime, static_cast<f64>(-1))};

  const auto modification_time{
      utils::filesystem::GetLastModificationTime(std::string{asset_abs_path})};

  return update_time <= modification_time;
}

bool AssetManager::IsRefreshNeeded(
    const schar* asset_abs_path, const std::string& metadata_file_path) const {
  return IsRefreshNeeded(asset_abs_path, metadata_file_path.c_str());
}

bool AssetManager::IsRefreshNeeded(const std::string& asset_abs_path,
                                   const schar* metadata_file_path) const {
  return IsRefreshNeeded(asset_abs_path.c_str(), metadata_file_path);
}

bool AssetManager::IsRefreshNeeded(
    const std::string& asset_abs_path,
    const std::string& metadata_file_path) const {
  return IsRefreshNeeded(asset_abs_path.c_str(), metadata_file_path);
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
