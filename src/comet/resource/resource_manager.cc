// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "resource_manager.h"

#include "comet/utils/date.h"
#include "comet/utils/file_system.h"
#include "efsw/efsw.hpp"
#include "nlohmann/json.hpp"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
void ResourceManager::Initialize() {
  InitializeResourcesDirectory();
  InitializeAssetsDirectory();

  Logger::Get(LoggerType::Resource)
      ->Debug("Resource manager listening to '", assets_root_path_, "'...");

  Refresh();
  InitializeWatcher();
  Watch();
}

void ResourceManager::Destroy() {}

void ResourceManager::InitializeAssetsDirectory() {
  assets_root_path_ = filesystem::Append(filesystem::GetCurrentDirectory(),
                                         kDefaultAssetsRootDirectory_);

  if (!filesystem::IsExist(assets_root_path_)) {
    filesystem::CreateDirectory(kDefaultAssetsRootDirectory_, true);
  }
}

void ResourceManager::InitializeResourcesDirectory() {
  resources_root_path_ = filesystem::Append(filesystem::GetCurrentDirectory(),
                                            kDefaultResourcesRootDirectory_);

  std::string library_meta_path =
      filesystem::Append(resources_root_path_, "library.meta");

  if (!filesystem::IsExist(resources_root_path_) ||
      !filesystem::IsFile(library_meta_path)) {
    filesystem::CreateDirectory(kDefaultResourcesRootDirectory_, true);
    last_update_time_ = -1;
  } else {
    std::string raw_library_meta_data;
    filesystem::ReadFile(library_meta_path, &raw_library_meta_data);
    auto library_meta_data = nlohmann::json::parse(raw_library_meta_data);
    last_update_time_ = library_meta_data["last_update_time"];
  }
}

void ResourceManager::handleFileAction(efsw::WatchID watch_id,
                                       const std::string &directory,
                                       const std::string &file_name,
                                       efsw::Action action,
                                       std::string old_file_name) {
  const auto logger = Logger::Get(LoggerType::Resource);

  auto path = filesystem::GetNormalizedPath(directory);
  path = filesystem::Append(path, filesystem::GetNormalizedPath(file_name));

  switch (action) {
    case efsw::Actions::Add:
      logger->Debug("Change detected: add ", path);
      break;

    case efsw::Actions::Delete:
      logger->Debug("Change detected: delete ", path);
      return;

    case efsw::Actions::Modified:
      logger->Debug("Change detected: modify ", path);
      break;

    case efsw::Actions::Moved:
      logger->Debug("Change detected: move ", path, " from ", old_file_name);
      break;

    default:
      logger->Error("Odd action '", action, "' detected with ", path);
      return;
  }

  Refresh(path);
}

void ResourceManager::Refresh() { Refresh(assets_root_path_); }

void ResourceManager::Refresh(const std::string &path) {
  Unwatch();

  if (filesystem::IsDirectory(path)) {
    RefreshFolder(path);
  } else if (filesystem::IsFile(path)) {
    RefreshAsset(path);
  } else {
    Logger::Get(LoggerType::Resource)->Error("Bad path given: ", path);

    Watch();

    return;
  }

  SetResourceMetaFile();
  Watch();
}

void ResourceManager::SetResourceMetaFile() {
  last_update_time_ = date::GetNow();

  const auto library_meta_file_path =
      filesystem::Append(resources_root_path_, "library.meta");

  nlohmann::json library_meta_data;

  library_meta_data["last_update_time"] = last_update_time_;
  std::string lirary_raw_meta_data = library_meta_data.dump(2);

  if (!filesystem::WriteToFile(library_meta_file_path, lirary_raw_meta_data)) {
    Logger::Get(LoggerType::Resource)
        ->Error("Could not write the resource meta file at path ",
                library_meta_file_path);
  }
}

void ResourceManager::SetFolderMetaFile(const std::string &path) {
  nlohmann::json folder_meta_data;
  const auto now = date::GetNow();

  if (filesystem::IsExist(path)) {
    std::string raw_meta_data;
    folder_meta_data = filesystem::ReadFile(path, &raw_meta_data);
    folder_meta_data = nlohmann::json::parse(raw_meta_data);
    folder_meta_data["modification_time"] = now;
  } else {
    folder_meta_data["creation_time"] = now;
    folder_meta_data["modification_time"] = now;
  }

  if (!filesystem::WriteToFile(path, folder_meta_data.dump(2))) {
    Logger::Get(LoggerType::Resource)
        ->Error("Could not write the folder meta file at path ", path);
  }
}

bool ResourceManager::IsRefreshFolder(const std::string &path,
                                      const std::string &meta_file_path) {
  if (path == assets_root_path_) return true;

  return last_update_time_ > filesystem::GetLastModificationTime(path) ||
         !filesystem::IsExist(meta_file_path);
}

void ResourceManager::RefreshFolder(const std::string &path) {
  const auto folder_name = filesystem::GetName(path);

  const auto meta_file_path = filesystem::Append(
      filesystem::GetParentPath(path), folder_name + ".dir.meta");

  if (!IsRefreshFolder(path, meta_file_path)) return;

  if (path != assets_root_path_) {
    SetFolderMetaFile(meta_file_path);
  }

  const auto folders = filesystem::ListDirectories(path);

  for (const auto &folder : folders) {
    RefreshFolder(folder);
  }

  const auto resources = filesystem::ListFiles(path);

  for (const auto &resource : resources) {
    RefreshAsset(resource);
  }

  Logger::Get(LoggerType::Resource)->Debug(path, " refreshed");
}

void ResourceManager::RefreshAsset(const std::string &path) {
  if (last_update_time_ > filesystem::GetLastModificationTime(path)) {
    return;
  }

  Logger::Get(LoggerType::Resource)->Debug(path, " refreshed");
}

void ResourceManager::InitializeWatcher() {
  assets_watcher_ = std::make_unique<efsw::FileWatcher>();
  assets_watcher_->watch();
}

void ResourceManager::Watch() {
  if (assets_watch_id_ != -1 || !assets_watcher_) return;

  assets_watch_id_ = assets_watcher_->addWatch(assets_root_path_, this, false);
}

void ResourceManager::Unwatch() {
  if (assets_watch_id_ == -1) return;

  assets_watcher_->removeWatch(assets_watch_id_);
  assets_watch_id_ = -1;
}

const std::string ResourceManager::assets_root_path() const noexcept {
  return assets_root_path_;
}

const std::string ResourceManager::resources_root_path() const noexcept {
  return resources_root_path_;
}
}  // namespace comet
