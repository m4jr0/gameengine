// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_RESOURCE_MANAGER_H_
#define COMET_COMET_RESOURCE_RESOURCE_MANAGER_H_

#include "comet_precompile.h"

#include "efsw/efsw.hpp"

#include "comet/core/manager.h"

namespace comet {
namespace resource {
class ResourceManager : public core::Manager, public efsw::FileWatchListener {
 public:
  static constexpr char kDefaultAssetsRootDirectory_[] = "assets";
  static constexpr char kDefaultResourcesRootDirectory_[] = "resources";

  ResourceManager() = default;
  ResourceManager(const ResourceManager&) = delete;
  ResourceManager(ResourceManager&&) = delete;
  ResourceManager& operator=(const ResourceManager&) = delete;
  ResourceManager& operator=(ResourceManager&&) = delete;
  virtual ~ResourceManager() = default;

  void Initialize() override;
  void Destroy() override;
  void Refresh();
  void Refresh(const std::string& path);
  void SetFolderMetaFile(const std::string& path);
  void SetResourceMetaFile();

  // Override esfw::FileWatchListener's method.
  void handleFileAction(efsw::WatchID watch_id, const std::string& directory,
                        const std::string& file_name, efsw::Action action,
                        std::string old_file_name = "") override;

  const std::string& GetAssetsRootPath() const noexcept;
  const std::string& GetResourcesRootPath() const noexcept;

 protected:
  double last_update_time_;
  std::string assets_root_path_;
  std::string resources_root_path_;
  std::unique_ptr<efsw::FileWatcher> assets_watcher_ = nullptr;
  efsw::WatchID assets_watch_id_ = -1;

  void RefreshFolder(const std::string& path);
  void RefreshAsset(const std::string& path);
  void Watch();
  void Unwatch();
  bool IsRefreshFolder(const std::string& path,
                       const std::string& meta_file_path);
  void InitializeAssetsDirectory();
  void InitializeResourcesDirectory();
  void InitializeWatcher();
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_RESOURCE_MANAGER_H_
