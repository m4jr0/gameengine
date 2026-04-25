// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_UTILS_SHADER_UTILS_H_
#define COMET_COMET_RENDERING_UTILS_SHADER_UTILS_H_

#include "comet/core/essentials.h"
#include "comet/rendering/type/shader.h"

namespace comet {
namespace rendering {
Alignment GetScalarAlignment(ShaderVariableType type);
Alignment GetStd140Alignment(ShaderVariableType type);
Alignment GetStd430Alignment(ShaderVariableType type);

ShaderVariableSize GetShaderVariableTypeSize(ShaderVariableType type);

void SetName(ShaderNamedDescr& descr, const schar* name, usize name_len);
void SetName(ShaderFieldDescr& descr, const schar* name, usize name_len);
void SetName(ShaderBindingDescr& descr, const schar* name, usize name_len);
void SetName(ShaderPushConstantDescr& descr, const schar* name, usize name_len);
void SetName(ShaderDefineDescr& descr, const schar* name, usize name_len);
void SetValue(ShaderDefineDescr& descr, const schar* value, usize value_len);

bool IsBufferBindingType(ShaderBindingType type);
bool IsImageBindingType(ShaderBindingType type);
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_UTILS_SHADER_UTILS_H_