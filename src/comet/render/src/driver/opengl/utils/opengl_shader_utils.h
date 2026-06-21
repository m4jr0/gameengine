// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_UTILS_OPENGL_SHADER_UTILS_H_
#define COMET_RENDER_DRIVER_OPENGL_UTILS_OPENGL_SHADER_UTILS_H_

#include "comet/core/essentials.h"
#include "comet/runtime/frame/frame_container.h"
#include "comet/render/driver/opengl/type/opengl_shader.h"
#include "comet/render/driver/opengl/type/opengl_texture_map.h"

namespace comet {
namespace render {
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

ShaderImageDescriptor GenerateImageDescriptor(ShaderBindingType binding_type,
                                              TextureHandle texture_handle,
                                              SamplerHandle sampler_handle);

void GenerateImageDescriptors(ShaderBindingType binding_type,
                              const Array<const TextureMap*>& texture_maps,
                              Array<ShaderImageDescriptor>& descriptors);

Alignment GetBindingFieldAlignment(ShaderMemoryLayout layout,
                                   ShaderVariableType type);

ShaderOffset AlignOffset(ShaderOffset offset, Alignment alignment);

ShaderFieldLayoutInfo GetFieldLayoutInfo(ShaderMemoryLayout layout,
                                         ShaderVariableType type,
                                         u32 array_count);

GLenum GetGlImageAccess(ShaderBindingType type);
GLenum GetGlCullMode(CullMode cull_mode);
GLenum GetGlPrimitiveTopology(PrimitiveTopology topology);
GLenum GetGlCompareOp(CompareOp compare_op);
GLenum GetGlStage(ShaderStage stage);
}  // namespace gl
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_OPENGL_UTILS_OPENGL_SHADER_UTILS_H_