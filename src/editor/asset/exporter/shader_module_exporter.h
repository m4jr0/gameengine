// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_SHADER_MODULE_EXPORTER_H_
#define COMET_EDITOR_ASSET_EXPORTER_SHADER_MODULE_EXPORTER_H_

#include "comet_precompile.h"

#include "comet/core/type/tstring.h"
#include "editor/asset/exporter/asset_exporter.h"

namespace comet {
namespace editor {
namespace asset {
class ShaderModuleExporter : public AssetExporter {
 public:
  ShaderModuleExporter() = default;
  ShaderModuleExporter(const ShaderModuleExporter&) = delete;
  ShaderModuleExporter(ShaderModuleExporter&&) = delete;
  ShaderModuleExporter& operator=(const ShaderModuleExporter&) = delete;
  ShaderModuleExporter& operator=(ShaderModuleExporter&&) = delete;
  virtual ~ShaderModuleExporter() = default;

  bool IsCompatible(CTStringView extension) const override;

 protected:
  std::vector<resource::ResourceFile> GetResourceFiles(
      AssetDescr& asset_descr) const override;
};
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_SHADER_MODULE_EXPORTER_H_
