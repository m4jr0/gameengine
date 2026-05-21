// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "tools/assetc/asset_manager.h"

#include <string>

#include "comet/core/file_system/file_system.h"
#include "comet/core/logger/logging.h"
#include "comet/core/type/tstring.h"
#include "nlohmann/json.hpp"
#include "tools/assetc/asset_utils.h"
#include "tools/assetc/exporter/model/model_exporter.h"
#include "tools/assetc/exporter/shader/shader_exporter.h"
#include "tools/assetc/exporter/shader/shader_module_exporter.h"
#include "tools/assetc/exporter/texture/texture_exporter.h"

namespace comet {
namespace tool {
namespace asset {

AssetProcessor::AssetProcessor(const AssetcConfig& config) : config_{config} {
  root_asset_path_ = config_.asset_root;
  Clean(root_asset_path_);

  root_resource_path_ = config_.resource_root;
  Clean(root_resource_path_);

  library_meta_path_.Reserve(root_asset_path_.GetLength() + 8 +
                             kCometAssetMetadataFileExtension.GetLength() + 1);
  library_meta_path_ = root_asset_path_;
  library_meta_path_ /= COMET_TCHAR("library.");
  library_meta_path_ += kCometAssetMetadataFileExtension;
  Clean(library_meta_path_);
}

AssetProcessor::~AssetProcessor() {
  if (is_initialized_) {
    ShutdownExporters();
  }

  root_asset_path_.Release();
  root_resource_path_.Release();
  library_meta_path_.Release();
}

bool AssetProcessor::Run() {
  InitializeExporters();
  Refresh();
  ShutdownExporters();
  return true;
}

void AssetProcessor::Initialize(const AssetcConfig& config) {
  COMET_ASSERT(!is_initialized_, "AssetProcessor::Initialize",
               "asset manager is already initialized");
  COMET_ASSERT(config.allocator != nullptr, "AssetProcessor::Initialize",
               "allocator is null");

  allocator_ = config.allocator;
  is_force_refresh_ = config.force;

  root_asset_path_ = config.asset_root;
  Clean(root_asset_path_);

  root_resource_path_ = config.resource_root;
  Clean(root_resource_path_);

  library_meta_path_ = root_asset_path_;
  library_meta_path_ /= COMET_TCHAR("library.");
  library_meta_path_ += kCometAssetMetadataFileExtension;
  Clean(library_meta_path_);

  RefreshLibraryMetadataFile();

  exporters_ = Array<memory::UniquePtr<AssetExporter>>{allocator_};
  exporters_.Reserve(4);
  exporters_.PushLast(std::make_unique<ModelExporter>());
  exporters_.PushLast(std::make_unique<ShaderExporter>());
  exporters_.PushLast(std::make_unique<ShaderModuleExporter>());
  exporters_.PushLast(std::make_unique<TextureExporter>());

  for (const auto& exporter : exporters_) {
    exporter->SetRootResourcePath(root_resource_path_);
    exporter->SetRootAssetPath(root_asset_path_);
  }

  is_initialized_ = true;
}

void AssetProcessor::Shutdown() {
  if (!is_initialized_) {
    return;
  }

  exporters_.Release();
  root_asset_path_.Release();
  root_resource_path_.Release();
  library_meta_path_.Release();

  allocator_ = nullptr;
  is_force_refresh_ = false;
  is_initialized_ = false;
}

void AssetProcessor::RefreshLibraryMetadataFile() {
  SaveMetadata(library_meta_path_, SetAndGetMetadata(library_meta_path_));
}

void AssetProcessor::Refresh() { RefreshLibrary(); }

const TString& AssetProcessor::GetAssetsRootPath() const noexcept {
  return root_asset_path_;
}

const TString& AssetProcessor::GetResourcesRootPath() const noexcept {
  return root_resource_path_;
}

void AssetProcessor::InitializeExporters() {
  if (is_initialized_) {
    return;
  }

  RefreshLibraryMetadataFile();

  exporters_ = Array<memory::UniquePtr<AssetExporter>>{&exporters_allocator_};
  exporters_.Reserve(4);
  exporters_.PushLast(std::make_unique<ModelExporter>());
  exporters_.PushLast(std::make_unique<ShaderExporter>());
  exporters_.PushLast(std::make_unique<ShaderModuleExporter>());
  exporters_.PushLast(std::make_unique<TextureExporter>());

  for (const auto& exporter : exporters_) {
    exporter->SetRootResourcePath(root_resource_path_);
    exporter->SetRootAssetPath(root_asset_path_);
  }

  is_initialized_ = true;
}

void AssetProcessor::ShutdownExporters() {
  is_initialized_ = false;
  exporters_.Release();
}

void AssetProcessor::RefreshLibrary() {
  RefreshFolder(root_asset_path_);
  RefreshLibraryMetadataFile();
}

void AssetProcessor::RefreshFolder(CTStringView asset_abs_path) {
  const auto parent_path{GetParentPath(asset_abs_path)};
  const auto folder_name{GetName(asset_abs_path)};

  if (folder_name.IsEmpty()) {
    COMET_LOG_ERROR(LoggerType::External, "AssetProcessor::RefreshFolder",
                    "folder name is empty", "asset_path", asset_abs_path);
    return;
  }

  TString metadata_file_path{};
  metadata_file_path.Reserve(
      parent_path.GetLength() + folder_name.GetLength() + 1 +
      kCometAssetFolderMetadataFileExtension.GetLength());
  metadata_file_path = parent_path;
  metadata_file_path /= folder_name;
  metadata_file_path += kCometAssetFolderMetadataFileExtension;

  if (asset_abs_path != root_asset_path_) {
    SetAndGetMetadata(metadata_file_path);
  }

  ForEachDirectory(asset_abs_path, [&](CTStringView directory_path) {
    RefreshFolder(directory_path);
  });

  ForEachFile(asset_abs_path,
              [&](CTStringView file_path) { RefreshAsset(file_path); });
}

void AssetProcessor::RefreshAsset(CTStringView asset_abs_path) {
  const auto asset_metadata_file_path{
      GenerateAssetMetadataFilePath(asset_abs_path)};

  if (IsMetadataFile(asset_abs_path) ||
      !IsRefreshNeeded(asset_abs_path, asset_metadata_file_path)) {
    return;
  }

  AssetExportDescr descr{};
  descr.asset_abs_path = asset_abs_path.GetCTStr();
  descr.allocator = allocator_;

  bool is_compatible{false};

  for (const auto& exporter : exporters_) {
    if (!exporter->IsCompatible(GetExtension(asset_abs_path))) {
      continue;
    }

    is_compatible = true;
    exporter->Process(descr);
  }

  if (!is_compatible) {
    COMET_LOG_WARNING(LoggerType::External, "AssetProcessor::RefreshAsset",
                      "no compatible exporter found", "asset_path",
                      asset_abs_path);
  }
}

bool AssetProcessor::IsRefreshNeeded(CTStringView asset_abs_path,
                                   CTStringView metadata_file_path) const {
  if (is_force_refresh_ || asset_abs_path == root_asset_path_ ||
      !Exists(metadata_file_path)) {
    return true;
  }

  const auto existing_metadata = GetMetadata(metadata_file_path);

  const auto update_time{existing_metadata.value(
      kCometAssetMetadataKeyUpdateTime, static_cast<f64>(-1))};

  const auto modification_time{GetLastModificationTime(asset_abs_path)};

  if (update_time <= modification_time) {
    return true;
  }

  if (!existing_metadata.contains(kCometAssetMetadataKeyResourceFiles)) {
    return true;
  }

  const auto& resource_files{
      existing_metadata[kCometAssetMetadataKeyResourceFiles]};

  if (!resource_files.is_array()) {
    COMET_LOG_WARNING(LoggerType::External, "AssetProcessor::IsRefreshNeeded",
                      "resource files metadata is not an array",
                      "metadata_path", metadata_file_path);
    return true;
  }

  TString item_abs_path{};
  item_abs_path.Reserve(kMaxPathLength);
  tchar item_path[kMaxPathLength + 1]{};

  for (const auto& item : resource_files) {
    if (!item.is_string()) {
      return true;
    }

    const auto& path_str{item.get_ref<const std::string&>()};

    if (path_str.size() > kMaxPathLength) {
      return true;
    }

    Copy(item_path, path_str.c_str(), path_str.size());
    item_path[path_str.size()] = COMET_TCHAR('\0');

    item_abs_path.Clear();
    item_abs_path /= root_resource_path_;
    item_abs_path /= item_path;

    if (!Exists(item_abs_path)) {
      return true;
    }
  }

  return false;
}

}  // namespace asset
}  // namespace tool
}  // namespace comet