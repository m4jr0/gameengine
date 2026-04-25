// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_UTILS_OPENGL_SHADER_UTILS_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_UTILS_OPENGL_SHADER_UTILS_H_

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_container.h"
#include "comet/rendering/driver/opengl/type/opengl_shader.h"
#include "comet/rendering/driver/opengl/type/opengl_shader_module.h"

namespace comet {
namespace rendering {
namespace gl {
bool IsGraphicsStage(ShaderStageFlags flags);
bool IsComputeStage(ShaderStageFlags flags);

u32 ResolveBinding(u32 set, u32 binding);

void AddBufferBinding(frame::FrameArray<ShaderBufferBindingUpdate>& updates,
                      ShaderBindingIndex binding_index, GLuint buffer_handle,
                      usize buffer_size, usize buffer_offset = 0);

void AddImageBinding(frame::FrameArray<ShaderImageBindingUpdate>& updates,
                     ShaderBindingIndex binding_index,
                     const ShaderImageDescriptor* descriptors,
                     u32 descriptor_count);

void AddFieldUpdate(frame::FrameArray<ShaderBufferFieldUpdate>& updates,
                    ShaderBindingIndex binding_index,
                    ShaderFieldIndex field_index, const void* data,
                    usize size = 0);

Alignment GetBindingFieldAlignment(ShaderMemoryLayout layout,
                                   ShaderVariableType type);

ShaderOffset AlignOffset(ShaderOffset offset, Alignment alignment);

ShaderFieldLayoutInfo GetFieldLayoutInfo(ShaderMemoryLayout layout,
                                         ShaderVariableType type,
                                         u32 array_count);

GlNativeProgramHandle ResolveProgramHandle(const Shader* shader,
                                           ShaderBindType bind_type);

GLenum GetGlImageAccess(ShaderBindingType type);
GLenum GetGlCullMode(CullMode cull_mode);
GLenum GetGlPrimitiveTopology(PrimitiveTopology topology);
GLenum GetGlCompareOp(CompareOp compare_op);
GLenum GetGlStage(ShaderStage stage);
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_UTILS_OPENGL_SHADER_UTILS_H_