// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "asset_manager.h"

#include "nlohmann/json.hpp"

#include <string>

#include "comet/core/c_string.h"
#include "comet/core/concurrency/job/job.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/file_system/file_system.h"
#include "comet/core/type/tstring.h"
#include "comet/resource/resource_manager.h"
#include "comet_pch.h"
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

  exporters_allocator_.Initialize();
  resource_file_allocator_.Initialize();
  exporters_ = Array<memory::UniquePtr<AssetExporter>>{&exporters_allocator_};
  exporters_.Reserve(4);
  exporters_.PushBack(std::make_unique<ModelExporter>());
  exporters_.PushBack(std::make_unique<ShaderExporter>());
  exporters_.PushBack(std::make_unique<ShaderModuleExporter>());
  exporters_.PushBack(std::make_unique<TextureExporter>());

  for (const auto& exporter : exporters_) {
    exporter->SetRootResourcePath(root_resource_path_);
    exporter->SetRootAssetPath(root_asset_path_);
  }
}

void AssetManager::RefreshLibraryMetadataFile() {
  SaveMetadata(library_meta_path_, SetAndGetMetadata(library_meta_path_));
}

void AssetManager::Shutdown() {
  is_force_refresh_ = false;
  last_update_time_ = 0;
  root_asset_path_.Destroy();
  root_resource_path_.Destroy();
  library_meta_path_.Destroy();
  exporters_.Destroy();
  exporters_allocator_.Destroy();
  resource_file_allocator_.Destroy();
  Manager::Shutdown();
}

void AssetManager::Refresh() {
  job::CounterGuard guard{};
  auto* counter{guard.GetCounter()};

  auto job{job::GenerateIOJobDescr(OnRefresh, counter, counter)};

  job::Scheduler::Get().Kick(job);
  guard.Wait();
}

const TString& AssetManager::GetAssetsRootPath() const noexcept {
  return root_asset_path_;
}

const TString& AssetManager::GetResourcesRootPath() const noexcept {
  return root_resource_path_;
}

void AssetManager::OnRefresh(job::IOJobParamsHandle params_handle) {
  auto* global_counter{reinterpret_cast<job::Counter*>(params_handle)};
  AssetManager::Get().RefreshLibrary(global_counter);
}

void AssetManager::RefreshLibrary(job::Counter* global_counter) {
  RefreshFolder(global_counter, root_asset_path_);
  RefreshLibraryMetadataFile();
}

void AssetManager::RefreshFolder(job::Counter* global_counter,
                                 CTStringView asset_abs_path) {
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
    RefreshFolder(global_counter, directory_path);
  });

  ForEachFile(asset_abs_path, [&](CTStringView file_path) {
    RefreshAsset(global_counter, file_path);
  });
}

void AssetManager::RefreshAsset(job::Counter* global_counter,
                                CTStringView asset_abs_path) {
  auto asset_metadata_file_path{GenerateAssetMetadataFilePath(asset_abs_path)};

  if (!IsRefreshNeeded(asset_abs_path, asset_metadata_file_path) ||
      IsMetadataFile(asset_abs_path)) {
    return;
  }

  AssetExportDescr descr{};
  descr.global_counter = global_counter;
  descr.asset_abs_path = asset_abs_path.GetCTStr();
  descr.allocator = &resource_file_allocator_;

  for (const auto& exporter : exporters_) {
    if (exporter->IsCompatible(GetExtension(asset_abs_path))) {
      exporter->Process(descr);
    }
  }
}

bool AssetManager::IsRefreshNeeded(CTStringView asset_abs_path,
                                   CTStringView metadata_file_path) const {
  if (is_force_refresh_ || asset_abs_path == root_asset_path_ ||
      !Exists(metadata_file_path)) {
    return true;
  }

  // We must use assignment here to prevent a bug with GCC where the generated
  // type is an array (which is wrong).
  const auto existing_metadata = GetMetadata(metadata_file_path);

  const auto update_time{existing_metadata.value(
      kCometEditorAssetMetadataKeyUpdateTime, static_cast<f64>(-1))};

  const auto modification_time{GetLastModificationTime(asset_abs_path)};

  if (update_time <= modification_time) {
    return true;
  }

  if (!existing_metadata.contains(kCometEditorAssetMetadataKeyResourceFiles)) {
    return true;
  }

  const auto& resource_files{
      existing_metadata[kCometEditorAssetMetadataKeyResourceFiles]};

  if (!resource_files.is_array()) {
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
}  // namespace editor
}  // namespace comet
