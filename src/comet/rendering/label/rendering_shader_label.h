// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_LABEL_RENDERING_SHADER_LABEL_H_
#define COMET_COMET_RENDERING_LABEL_RENDERING_SHADER_LABEL_H_

#include "comet/core/essentials.h"
#include "comet/rendering/type/rendering_shader_type.h"

namespace comet {
namespace rendering {
const schar* GetShaderStageLabel(ShaderStage stage);
const schar* GetShaderVariableTypeLabel(ShaderVariableType type);
const schar* GetShaderBindingTypeLabel(ShaderBindingType type);
const schar* GetShaderBindingScopeLabel(ShaderBindingScope scope);
const schar* GetShaderMemoryLayoutLabel(ShaderMemoryLayout layout);
const schar* GetShaderVertexLayoutLabel(ShaderVertexLayout layout);
const schar* GetShaderImageBindingSemanticLabel(
    ShaderImageBindingSemantic semantic);
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_LABEL_RENDERING_SHADER_LABEL_H_