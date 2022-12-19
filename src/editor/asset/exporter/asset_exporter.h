// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_ASSET_EXPORTER_H_
#define COMET_EDITOR_ASSET_EXPORTER_ASSET_EXPORTER_H_

#include "comet_precompile.h"

#include "editor/asset/asset.h"

namespace comet {
namespace editor {
namespace asset {
class AssetExporter {
 public:
  AssetExporter() = default;
  AssetExporter(const AssetExporter&) = delete;
  AssetExporter(AssetExporter&&) = delete;
  AssetExporter& operator=(const AssetExporter&) = delete;
  AssetExporter& operator=(AssetExporter&&) = delete;
  virtual ~AssetExporter() = default;

  virtual bool IsCompatible(std::string_view extension) const = 0;
  bool Process(std::string_view asset_path);

  template <typename ResourcePath>
  void SetRootResourcePath(ResourcePath&& path) {
    root_resource_path_ = std::forward<ResourcePath>(path);
  };

  template <typename AssetPath>
  void SetRootAssetPath(AssetPath&& path) {
    root_asset_path_ = std::forward<AssetPath>(path);
  };

  const std::string& GetRootResourcePath() const;
  const std::string& GetRootAssetPath() const;

 protected:
  virtual std::vector<resource::ResourceFile> GetResourceFiles(
      AssetDescr& asset_descr) const = 0;

  resource::CompressionMode compression_mode_{resource::CompressionMode::Lz4};
  std::string root_asset_path_{};
  std::string root_resource_path_{};
};
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_ASSET_EXPORTER_H_
