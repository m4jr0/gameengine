// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_ASSET_MANAGER_HELPER_H_
#define COMET_EDITOR_ASSET_ASSET_MANAGER_HELPER_H_

#include "comet/core/concurrency/fiber/fiber_primitive.h"  // >:3
#include "comet/core/concurrency/job/job.h"
#include "comet/core/essentials.h"
#include "comet/core/manager.h"
#include "editor/asset/exporter/asset_exporter.h"

namespace comet {
namespace editor {
namespace asset {
namespace internal {
class AssetManagerHelper {
 public:
  static job::JobDescr GenerateItemRefreshJobDescr(CTStringView path,
                                                   job::Counter* counter,
                                                   bool is_folder);
  static job::JobDescr GenerateAssetExportJobDescr(CTStringView asset_path,
                                                   AssetExporter* exporter,
                                                   job::Counter* counter);

 private:
  static inline fiber::FiberMutex mutex_{};

  struct RefreshItemDescr {
    static constexpr usize kBufferLen{kMaxPathLength + 1};

    bool is_folder{false};
    tchar path[kBufferLen]{COMET_TCHAR('\0')};
    job::Counter* counter{nullptr};
  };

  struct AssetExportDescr {
    static constexpr usize kBufferLen{kMaxPathLength + 1};

    tchar asset_path[kBufferLen]{COMET_TCHAR('\0')};
    AssetExporter* exporter{nullptr};
  };

  static void OnItemRefresh(fiber::ParamsHandle params_handle);

  static void OnAssetExport(fiber::ParamsHandle params_handle);
};
}  // namespace internal
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_ASSET_MANAGER_HELPER_H_
