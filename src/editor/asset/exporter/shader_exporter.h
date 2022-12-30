// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_SHADER_EXPORTER_H_
#define COMET_EDITOR_ASSET_EXPORTER_SHADER_EXPORTER_H_

#include "comet_precompile.h"

#include "editor/asset/exporter/asset_exporter.h"

namespace comet {
namespace editor {
namespace asset {
class ShaderExporter : public AssetExporter {
 public:
  ShaderExporter() = default;
  ShaderExporter(const ShaderExporter&) = delete;
  ShaderExporter(ShaderExporter&&) = delete;
  ShaderExporter& operator=(const ShaderExporter&) = delete;
  ShaderExporter& operator=(ShaderExporter&&) = delete;
  ~ShaderExporter() = default;

  bool IsCompatible(const std::string& extension) override;

 protected:
  std::vector<resource::ResourceFile> GetResourceFiles(
      AssetDescr& asset_descr) override;
};
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_SHADER_EXPORTER_H_
