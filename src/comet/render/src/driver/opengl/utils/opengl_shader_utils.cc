// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/opengl/utils/opengl_shader_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/type_trait.h"
#include "comet/rendering/label/pipeline_label.h"
#include "comet/rendering/label/shader_label.h"
#include "comet/rendering/utils/shader_utils.h"

namespace comet {
namespace rendering {
namespace gl {
bool IsGraphicsStage(ShaderStageFlags flags) {
  return (flags &
          (kShaderStageFlagBitsVertex | kShaderStageFlagBitsFragment)) != 0;
}

bool IsComputeStage(ShaderStageFlags flags) {
  return (flags & kShaderStageFlagBitsCompute) != 0;
}

u32 ResolveBinding(u32 set, u32 binding) {
  switch (set) {
    case shaderconsts::kGlobalSet:
      return binding + shaderconsts::kGlobalSetOffset;
    case shaderconsts::kMaterialSet:
      return binding + shaderconsts::kMaterialSetOffset;
    case shaderconsts::kPassSet:
      return binding + shaderconsts::kPassSetOffset;
    default:
      COMET_ASSERT(false, "opengl_shader_utils::ResolveBinding",
                   "logical set is unsupported", "set", set);
      return 0;
  }
}

void AddBufferBinding(frame::FrameArray<ShaderBufferBindingUpdate>& updates,
                      ShaderBindingIndex binding_index, GLuint buffer_handle,
                      usize buffer_size, usize buffer_offset) {
  auto& update{updates.EmplaceLast()};
  update.binding_index = binding_index;
  update.buffer_handle = buffer_handle;
  update.buffer_size = buffer_size;
  update.buffer_offset = buffer_offset;
}

void AddImageBinding(frame::FrameArray<ShaderImageBindingUpdate>& updates,
                     ShaderBindingIndex binding_index,
                     const ShaderImageDescriptor* descriptors,
                     u32 descriptor_count) {
  auto& update{updates.EmplaceLast()};
  update.binding_index = binding_index;
  update.descriptors = descriptors;
  update.descriptor_count = descriptor_count;
}

void AddFieldUpdate(frame::FrameArray<ShaderBufferFieldUpdate>& updates,
                    ShaderBindingIndex binding_index,
                    ShaderFieldIndex field_index, const void* data,
                    usize size) {
  auto& update{updates.EmplaceLast()};
  update.binding_index = binding_index;
  update.field_index = field_index;
  update.data = data;
  update.size = size;
}

ShaderImageDescriptor GenerateImageDescriptor(ShaderBindingType binding_type,
                                              TextureHandle texture_handle,
                                              SamplerHandle sampler_handle) {
  ShaderImageDescriptor descriptor{};

  switch (binding_type) {
    case ShaderBindingType::CombinedImageSampler:
      COMET_ASSERT(texture_handle,
                   "opengl_shader_utils::GenerateImageDescriptor",
                   "combined image sampler requires texture");
      COMET_ASSERT(sampler_handle,
                   "opengl_shader_utils::GenerateImageDescriptor",
                   "combined image sampler requires sampler");

      descriptor.texture_handle = texture_handle;
      descriptor.sampler_handle = sampler_handle;
      return descriptor;

    case ShaderBindingType::SampledImage:
      COMET_ASSERT(texture_handle,
                   "opengl_shader_utils::GenerateImageDescriptor",
                   "sampled image requires texture");

      descriptor.texture_handle = texture_handle;
      descriptor.sampler_handle = {};
      return descriptor;

    case ShaderBindingType::Sampler:
      COMET_ASSERT(sampler_handle,
                   "opengl_shader_utils::GenerateImageDescriptor",
                   "sampler binding requires sampler");

      descriptor.texture_handle = {};
      descriptor.sampler_handle = sampler_handle;
      return descriptor;

    case ShaderBindingType::StorageImage:
      COMET_ASSERT(texture_handle,
                   "opengl_shader_utils::GenerateImageDescriptor",
                   "storage image requires texture");

      descriptor.texture_handle = texture_handle;
      descriptor.sampler_handle = {};
      return descriptor;

    default:
      COMET_ASSERT(false, "opengl_shader_utils::GenerateImageDescriptor",
                   "image binding type is unsupported", "binding_type",
                   GetShaderBindingTypeLabel(binding_type),
                   "binding_type_value", ToUnderlying(binding_type));

      return descriptor;
  }
}

void GenerateImageDescriptors(ShaderBindingType binding_type,
                              const Array<const TextureMap*>& texture_maps,
                              Array<ShaderImageDescriptor>& descriptors) {
  COMET_ASSERT(IsImageBindingType(binding_type),
               "opengl_shader_utils::GenerateImageDescriptors",
               "binding type is not an image binding", "binding_type",
               GetShaderBindingTypeLabel(binding_type), "binding_type_value",
               ToUnderlying(binding_type));

  descriptors.Clear();
  descriptors.Reserve(texture_maps.GetSize());

  for (usize i{0}; i < texture_maps.GetSize(); ++i) {
    const auto* texture_map{texture_maps[i]};
    COMET_ASSERT(texture_map != nullptr,
                 "opengl_shader_utils::GenerateImageDescriptors",
                 "texture map is null", "index", i);

    descriptors.PushLast(GenerateImageDescriptor(binding_type,
                                                 texture_map->texture_handle,
                                                 texture_map->sampler_handle));
  }
}

GLenum GetGlImageAccess(ShaderBindingType type) {
  switch (type) {
    case ShaderBindingType::StorageImage:
      return GL_READ_WRITE;
    default:
      return GL_READ_ONLY;
  }
}

Alignment GetBindingFieldAlignment(ShaderMemoryLayout layout,
                                   ShaderVariableType type) {
  switch (layout) {
    case ShaderMemoryLayout::Std140:
      return GetStd140Alignment(type);
    case ShaderMemoryLayout::Std430:
      return GetStd430Alignment(type);
    case ShaderMemoryLayout::Packed:
      return GetScalarAlignment(type);
    default:
      COMET_ASSERT(false, "opengl_shader_utils::GetBindingFieldAlignment",
                   "shader memory layout is invalid", "layout",
                   GetShaderMemoryLayoutLabel(layout), "layout_value",
                   ToUnderlying(layout));
  }

  return kInvalidAlignment;
}

ShaderOffset AlignOffset(ShaderOffset offset, Alignment alignment) {
  return static_cast<ShaderOffset>(
      memory::AlignSize(offset, static_cast<memory::Alignment>(alignment)));
}

ShaderFieldLayoutInfo GetFieldLayoutInfo(ShaderMemoryLayout layout,
                                         ShaderVariableType type,
                                         u32 array_count) {
  ShaderFieldLayoutInfo info{};

  info.element_size = GetShaderVariableTypeSize(type);
  COMET_ASSERT(info.element_size != kInvalidShaderVariableSize,
               "opengl_shader_utils::GetFieldLayoutInfo",
               "shader variable type size is invalid", "type",
               GetShaderVariableTypeLabel(type), "type_value",
               ToUnderlying(type));

  info.alignment = GetBindingFieldAlignment(layout, type);

  info.aligned_size = static_cast<ShaderVariableSize>(memory::AlignSize(
      info.element_size, static_cast<memory::Alignment>(info.alignment)));

  info.stride = info.aligned_size;

  if (array_count > 1) {
    info.total_size =
        static_cast<ShaderVariableSize>(info.stride * array_count);
  } else {
    info.total_size = info.aligned_size;
  }

  return info;
}

GLenum GetGlCullMode(CullMode cull_mode) {
  switch (cull_mode) {
    case CullMode::None:
      return GL_NONE;
    case CullMode::Front:
      return GL_FRONT;
    case CullMode::Back:
      return GL_BACK;
    case CullMode::FrontAndBack:
      return GL_FRONT_AND_BACK;
    default:
      COMET_ASSERT(false, "opengl_shader_utils::GetGlCullMode",
                   "cull mode is unsupported", "cull_mode",
                   GetCullModeLabel(cull_mode), "cull_mode_value",
                   ToUnderlying(cull_mode));
      return 0;
  }
}

GLenum GetGlPrimitiveTopology(PrimitiveTopology topology) {
  switch (topology) {
    case PrimitiveTopology::Points:
      return GL_POINTS;
    case PrimitiveTopology::Lines:
      return GL_LINES;
    case PrimitiveTopology::LineStrip:
      return GL_LINE_STRIP;
    case PrimitiveTopology::Triangles:
      return GL_TRIANGLES;
    case PrimitiveTopology::TriangleStrip:
      return GL_TRIANGLE_STRIP;
    default:
      COMET_ASSERT(false, "opengl_shader_utils::GetGlPrimitiveTopology",
                   "primitive topology is unsupported", "topology",
                   GetPrimitiveTopologyLabel(topology), "topology_value",
                   ToUnderlying(topology));
      return 0;
  }
}

GLenum GetGlCompareOp(CompareOp compare_op) {
  switch (compare_op) {
    case CompareOp::Never:
      return GL_NEVER;
    case CompareOp::Less:
      return GL_LESS;
    case CompareOp::Equal:
      return GL_EQUAL;
    case CompareOp::LessOrEqual:
      return GL_LEQUAL;
    case CompareOp::Greater:
      return GL_GREATER;
    case CompareOp::NotEqual:
      return GL_NOTEQUAL;
    case CompareOp::GreaterOrEqual:
      return GL_GEQUAL;
    case CompareOp::Always:
      return GL_ALWAYS;
    default:
      COMET_ASSERT(false, "opengl_shader_utils::GetGlCompareOp",
                   "compare op is unsupported", "compare_op",
                   ToUnderlying(compare_op));
      return 0;
  }
}

GLenum GetGlStage(ShaderStage stage) {
  switch (stage) {
    case ShaderStage::Compute:
      return GL_COMPUTE_SHADER;

    case ShaderStage::Vertex:
      return GL_VERTEX_SHADER;

    case ShaderStage::Fragment:
      return GL_FRAGMENT_SHADER;

    default:
      COMET_ASSERT(false, "opengl_shader_utils::GetGlStage",
                   "shader stage is invalid", "stage", ToUnderlying(stage));
      return GL_INVALID_VALUE;
  }
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet