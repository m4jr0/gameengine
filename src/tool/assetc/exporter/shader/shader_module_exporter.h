// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_SHADER_SHADER_MODULE_EXPORTER_H_
#define COMET_EDITOR_ASSET_EXPORTER_SHADER_SHADER_MODULE_EXPORTER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "shaderc/shaderc.hpp"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core.h"
#include "exporter/asset_exporter.h"

namespace comet {
namespace tool {
namespace assetc {
class ShaderModuleExporter : public AssetExporter {
 public:
  ShaderModuleExporter() = default;
  ShaderModuleExporter(const ShaderModuleExporter&) = delete;
  ShaderModuleExporter(ShaderModuleExporter&&) = delete;
  ShaderModuleExporter& operator=(const ShaderModuleExporter&) = delete;
  ShaderModuleExporter& operator=(ShaderModuleExporter&&) = delete;
  ~ShaderModuleExporter() override = default;

  bool IsCompatible(CTStringView extension) const override;

 protected:
  struct ShaderCodeContext {
    static inline constexpr usize kMaxShaderCodeLen_{32768};

    schar* code{nullptr};
    usize code_len;
    const tchar* asset_abs_path{nullptr};
    memory::Allocator* allocator{nullptr};
  };

  void PopulateFiles(ResourceFilesContext& context) const override;

 private:
  static void OnShaderModuleLoading(job::IOJobParamsHandle params_handle);
};
}  // namespace assetc
}  // namespace tool
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_SHADER_SHADER_MODULE_EXPORTER_H_
