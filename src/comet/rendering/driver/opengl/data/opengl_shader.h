// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_SHADER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_SHADER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/rendering/driver/opengl/data/opengl_pipeline.h"
#include "comet/rendering/driver/opengl/data/opengl_shader_data.h"
#include "comet/rendering/driver/opengl/data/opengl_shader_module.h"
#include "comet/rendering/rendering_handle.h"
#include "comet/rendering/rendering_type.h"
#include "comet/resource/shader_resource.h"

namespace comet {
namespace rendering {
namespace gl {
using GlNativeProgramHandle = GLuint;
constexpr auto kInvalidGlNativeProgramHandle{
    static_cast<GlNativeProgramHandle>(0)};

using GlNativeVertexAttributeHandle = GLuint;
constexpr auto kInvalidGlNativeVertexAttributeHandle{0};

struct ShaderKey {
  resource::ShaderResourceId shader_resource_id{};
  RenderPassHandle render_pass_handle{};

  friend constexpr bool operator==(const ShaderKey& lhs,
                                   const ShaderKey& rhs) noexcept = default;
};

using VertexAttributeStride = s32;

struct VertexAttribute {
  GLuint index{0};
  GLint component_count{0};
  GLenum component_type{0};
  GLboolean is_normalized{GL_FALSE};
  GLsizei stride{0};
  const void* offset{nullptr};
};

struct ShaderDescr {
  resource::ShaderResourceId shader_resource_id{};
};

struct Shader {
  ShaderHandle handle{};
  RasterizerState rasterizer{};
  DepthStencilState depth_stencil{};
  GLenum topology{0};
  ShaderVertexLayout vertex_layout{ShaderVertexLayout::None};

  resource::ShaderResourceId id{};
  bool has_vertex_source_binding{false};
  u64 vertex_source_id{0};

  VertexAttributeStride vertex_attribute_stride{0};

  sptrdiff bound_global_ubo_offset{0};
  sptrdiff bound_instance_ubo_offset{0};

  GlNativeProgramHandle compute_program_native_handle{
      kInvalidGlNativeProgramHandle};
  GlNativeProgramHandle graphics_program_native_handle{
      kInvalidGlNativeProgramHandle};

  GlNativeVertexAttributeHandle vertex_attribute_native_handle{
      kInvalidGlNativeVertexAttributeHandle};
  GlNativeUniformBufferHandle uniform_buffer_native_handle{
      kInvalidGlNativeUniformBufferHandle};

  Array<VertexAttribute> vertex_attributes{};
  Array<ShaderBinding> bindings{};
  Array<ShaderPushConstantBlock> push_constant_blocks{};
  Array<ShaderModuleHandle> module_handles{};

  ShaderUniformBufferObjectData global_ubo_data{};
  ShaderUniformBufferObjectData instance_ubo_data{};

  ShaderDescriptorSetRuntimeData global_descriptor_data{};
  ShaderDescriptorSetRuntimeData storage_descriptor_data{};

  MaterialInstances instances{};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_SHADER_H_