// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_SHADER_SHADER_MODULE_EXPORT_UTILS_H_
#define COMET_EDITOR_ASSET_EXPORTER_SHADER_SHADER_MODULE_EXPORT_UTILS_H_

// External. ///////////////////////////////////////////////////////////////////
#include "shaderc/shaderc.hpp"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core.h"
#include "comet/data.h"

namespace comet {
namespace tool {
namespace assetc {
bool PopulateSpvShaderCode(CTStringView asset_abs_path, schar* code,
                           memory::Allocator* allocator,
                           resource::ShaderModuleResource& shader_module);
bool PopulateGlShaderCode(schar* code, usize code_len,
                          memory::Allocator* allocator,
                          resource::ShaderModuleResource& shader_module);

void AddSpvMacroDefinitions(shaderc::CompileOptions& options);
}  // namespace assetc
}  // namespace tool
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_SHADER_SHADER_MODULE_EXPORT_UTILS_H_