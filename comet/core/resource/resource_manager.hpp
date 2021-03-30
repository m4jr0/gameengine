// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_RESOURCE_RESOURCE_MANAGER_HPP_
#define COMET_CORE_RESOURCE_RESOURCE_MANAGER_HPP_

constexpr auto kLoggerCometCoreResourceResourceManager = "comet_core_resource";

#include <filesystem>
#include <string>

#include "core/manager.hpp"
#include "efsw/efsw.hpp"

namespace comet {
class ResourceManager : public Manager, public efsw::FileWatchListener {
 public:
  static constexpr char kDefaultAssetsRootDirectory_[] = "assets";
  static constexpr char kDefaultResourcesRootDirectory_[] = "resources";

  ResourceManager() = default;
  ResourceManager(const ResourceManager &) = delete;
  ResourceManager(ResourceManager &&) = delete;
  ResourceManager &operator=(const ResourceManager &) = delete;
  ResourceManager &operator=(ResourceManager &&) = delete;
  virtual ~ResourceManager() = default;

  void Initialize() override;
  void Destroy() override;
  void Refresh();
  void Refresh(const std::string &);
  void SetFolderMetaFile(const std::string &);
  void SetResourceMetaFile();

  // Override esfw::FileWatchListener's method.
  void handleFileAction(efsw::WatchID, const std::string &, const std::string &,
                        efsw::Action, std::string = "") override;

  const std::string assets_root_path() const noexcept;
  const std::string resources_root_path() const noexcept;

 protected:
  double last_update_time_;
  std::string assets_root_path_;
  std::string resources_root_path_;
  std::unique_ptr<efsw::FileWatcher> assets_watcher_ = nullptr;
  efsw::WatchID assets_watch_id_ = -1;

  void RefreshFolder(const std::string &);
  void RefreshAsset(const std::string &);
  void Watch();
  void Unwatch();
  bool IsRefreshFolder(const std::string &, const std::string &);

  void InitializeAssetsDirectory();
  void InitializeResourcesDirectory();
  void InitializeWatcher();
};
}  // namespace comet

#endif  // COMET_CORE_RESOURCE_RESOURCE_MANAGER_HPP_
