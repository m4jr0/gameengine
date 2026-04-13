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
#include "comet/rendering/rendering_common.h"
#include "comet/resource/resource.h"

namespace comet {
namespace rendering {
namespace gl {
using ShaderHandle = u32;
constexpr auto kInvalidShaderHandle{static_cast<ShaderHandle>(-1)};

using VertexAttributeHandle = GLuint;
constexpr auto kInvalidVertexAttributeHandle{0};

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
  resource::ResourceId shader_id{resource::kInvalidResourceId};
};

struct Shader {
  RasterizerState rasterizer{};
  DepthStencilState depth_stencil{};
  GLenum topology{0};
  ShaderVertexLayout vertex_layout{ShaderVertexLayout::None};

  resource::ResourceId id{resource::kInvalidResourceId};
  ShaderHandle handle{kInvalidShaderHandle};
  u32 ref_count{0};

  bool has_vertex_source_binding{false};
  u64 vertex_source_id{0};

  VertexAttributeStride vertex_attribute_stride{0};

  sptrdiff bound_global_ubo_offset{0};
  sptrdiff bound_instance_ubo_offset{0};

  ShaderHandle compute_handle{kInvalidShaderHandle};
  ShaderHandle graphics_handle{kInvalidShaderHandle};

  VertexAttributeHandle vertex_attribute_handle{kInvalidVertexAttributeHandle};
  UniformBufferHandle uniform_buffer_handle{kInvalidUniformBufferHandle};

  Array<VertexAttribute> vertex_attributes{};
  Array<ShaderBinding> bindings{};
  Array<ShaderPushConstantBlock> push_constant_blocks{};
  Array<const ShaderModule*> modules{};

  ShaderUniformBufferObjectData global_ubo_data{};
  ShaderUniformBufferObjectData instance_ubo_data{};

  ShaderDescriptorSetRuntimeData global_descriptor_data{};
  ShaderDescriptorSetRuntimeData storage_descriptor_data{};

  MaterialInstances instances{};
};

ShaderHandle ResolveHandle(const Shader* shader, ShaderBindType bind_type);
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_SHADER_H_