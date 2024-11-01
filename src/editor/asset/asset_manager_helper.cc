// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "asset_manager_helper.h"

#include "comet/core/concurrency/fiber/fiber.h"
#include "comet/core/concurrency/job/job.h"
#include "comet/core/concurrency/job/scheduler_utils.h"
#include "comet/core/memory/tagged_heap.h"
#include "editor/asset/asset_manager.h"

namespace comet {
namespace editor {
namespace asset {
namespace internal {
job::JobDescr AssetManagerHelper::GenerateItemRefreshJobDescr(
    CTStringView path, job::Counter* counter, bool is_folder) {
  COMET_ASSERT(counter != nullptr, "Counter provided is null!");
  COMET_ASSERT(path.GetLength() > 0, "Path provided is empty!");
  COMET_ASSERT(path.GetLength() < RefreshItemDescr::kBufferLen,
               "Path provided is too long!");

  RefreshItemDescr* descr{nullptr};

  {
    fiber::FiberLockGuard lock{mutex_};

    descr =
        reinterpret_cast<RefreshItemDescr*>(memory::TaggedHeap::Get().Allocate(
            sizeof(RefreshItemDescr), memory::MemoryTag::Asset));
  }

  COMET_ASSERT(descr != nullptr,
               "Could not allocate for AssetExportDescr instance!");
  descr->is_folder = is_folder;
  Copy(descr->path, path.GetCTStr(), path.GetLengthWithNullTerminator());
  descr->counter = counter;

  return job::GenerateJobDescr(job::JobPriority::Normal, OnItemRefresh,
                               reinterpret_cast<fiber::ParamsHandle>(descr),
                               job::JobStackSize::Normal, counter);
}

job::JobDescr AssetManagerHelper::GenerateAssetExportJobDescr(
    CTStringView asset_path, AssetExporter* exporter, job::Counter* counter) {
  COMET_ASSERT(counter != nullptr, "Counter provided is null!");
  COMET_ASSERT(asset_path.GetLength() > 0, "Path provided is empty!");
  COMET_ASSERT(asset_path.GetLength() < AssetExportDescr::kBufferLen,
               "Path provided is too long!");

  AssetExportDescr* descr{nullptr};

  {
    fiber::FiberLockGuard lock{mutex_};
    descr =
        reinterpret_cast<AssetExportDescr*>(memory::TaggedHeap::Get().Allocate(
            sizeof(AssetExportDescr), memory::MemoryTag::Asset));
  }

  COMET_ASSERT(descr != nullptr,
               "Could not allocate for AssetExportDescr instance!");
  Copy(descr->asset_path, asset_path.GetCTStr(),
       asset_path.GetLengthWithNullTerminator());
  descr->exporter = exporter;

  return job::GenerateJobDescr(job::JobPriority::Normal, OnAssetExport,
                               reinterpret_cast<fiber::ParamsHandle>(descr),
                               job::JobStackSize::Normal, counter);
}

void AssetManagerHelper::OnItemRefresh(fiber::ParamsHandle params_handle) {
  auto* descr{reinterpret_cast<RefreshItemDescr*>(params_handle)};
  auto& asset_manager{AssetManager::Get()};

  if (descr->is_folder) {
    asset_manager.RefreshFolder(descr->path, descr->counter);
    return;
  }

  asset_manager.RefreshAsset(descr->path, descr->counter);
}

void AssetManagerHelper::OnAssetExport(fiber::ParamsHandle params_handle) {
  auto* descr{reinterpret_cast<AssetExportDescr*>(params_handle)};
  descr->exporter->Process(descr->asset_path);
}
}  // namespace internal
}  // namespace asset
}  // namespace editor
}  // namespace comet
