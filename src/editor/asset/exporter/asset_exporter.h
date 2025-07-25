// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_ASSET_EXPORTER_H_
#define COMET_EDITOR_ASSET_EXPORTER_ASSET_EXPORTER_H_

#include "comet/core/concurrency/job/job.h"
#include "comet/core/essentials.h"
#include "comet/core/file_system/file_system.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/type/array.h"
#include "comet/core/type/tstring.h"
#include "comet/resource/resource.h"
#include "editor/asset/asset.h"
#include "editor/memory/memory.h"

namespace comet {
namespace editor {
namespace asset {
struct AssetExportDescr {
  job::Counter* global_counter{nullptr};
  const tchar* asset_abs_path{nullptr};
  memory::Allocator* allocator{nullptr};
};

using ResourceFiles = Array<resource::ResourceFile>;

struct ResourceFilesContext {
  AssetDescr asset_descr{};
  job::Counter* global_counter{nullptr};
  memory::Allocator* allocator{nullptr};
  ResourceFiles files{};
};

class AssetExporter;

struct AssetExport {
  AssetExporter* exporter{nullptr};
  ResourceFilesContext context{};
};

class AssetExporter {
 public:
  AssetExporter() = default;
  AssetExporter(const AssetExporter&) = delete;
  AssetExporter(AssetExporter&&) = delete;
  AssetExporter& operator=(const AssetExporter&) = delete;
  AssetExporter& operator=(AssetExporter&&) = delete;
  virtual ~AssetExporter() = default;

  virtual bool IsCompatible(CTStringView extension) const = 0;
  void Process(const AssetExportDescr& descr);

  template <typename ResourcePath>
  void SetRootResourcePath(ResourcePath&& path) {
    root_resource_path_ = std::forward<ResourcePath>(path);
    Clean(root_resource_path_.GetTStr(), root_resource_path_.GetLength());
  }

  template <typename AssetPath>
  void SetRootAssetPath(AssetPath&& path) {
    root_asset_path_ = std::forward<AssetPath>(path);
    Clean(root_asset_path_.GetTStr(), root_asset_path_.GetLength());
  }

  const TString& GetRootResourcePath() const;
  const TString& GetRootAssetPath() const;

 protected:
  static void OnResourceFilesProcess(job::JobParamsHandle params_handle);
  static void OnResourceFilesWrite(job::IOJobParamsHandle params_handle);

  virtual void PopulateFiles(ResourceFilesContext& context) const = 0;

  resource::CompressionMode compression_mode_{resource::CompressionMode::Lz4};
  TString root_asset_path_{};
  TString root_resource_path_{};

 private:
  void OnAssetProcessed(AssetExport* asset_export);
  AssetExport* GenerateAssetExport();
  void DestroyAssetExport(AssetExport* asset_export);

  memory::PlatformAllocator asset_export_allocator_{
      memory::kEditorMemoryTagAsset};
};
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_ASSET_EXPORTER_H_
