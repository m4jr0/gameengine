// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_SHADER_SHADER_MODULE_EXPORTER_H_
#define COMET_EDITOR_ASSET_EXPORTER_SHADER_SHADER_MODULE_EXPORTER_H_

#include "comet/core/concurrency/job/job.h"
#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
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
  void PopulateFiles(ResourceFilesContext& context) const override;

 private:
  struct ShaderCodeContext {
    static inline constexpr usize kMaxShaderCodeLen_{4095};

    schar code[kMaxShaderCodeLen_ + 1];
    usize code_len;
    const tchar* asset_abs_path{nullptr};
  };

  static void OnShaderModuleLoading(job::IOJobParamsHandle params_handle);
};
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_SHADER_SHADER_MODULE_EXPORTER_H_
