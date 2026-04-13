// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_common_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/rendering/driver/opengl/data/opengl_shader_data.h"

namespace comet {
namespace rendering {
namespace gl {
u32 ResolveBinding(u32 set, u32 binding) {
  switch (set) {
    case shaderconsts::kGlobalSet:
      return binding + shaderconsts::kGlobalSetOffset;
    case shaderconsts::kMaterialSet:
      return binding + shaderconsts::kMaterialSetOffset;
    case shaderconsts::kPassSet:
      return binding + shaderconsts::kPassSetOffset;
    default:
      COMET_ASSERT(false, "Unsupported OpenGL logical set: ", set, "!");
      return 0;
  }
}

void AddBufferBinding(frame::FrameArray<ShaderBufferBindingUpdate>& updates,
                      ShaderBindingIndex binding_index, GLuint buffer_handle,
                      usize buffer_size, usize buffer_offset) {
  auto& update{updates.EmplaceBack()};
  update.binding_index = binding_index;
  update.buffer_handle = buffer_handle;
  update.buffer_size = buffer_size;
  update.buffer_offset = buffer_offset;
}

void AddImageBinding(frame::FrameArray<ShaderImageBindingUpdate>& updates,
                     ShaderBindingIndex binding_index,
                     const ShaderImageDescriptor* descriptors,
                     u32 descriptor_count) {
  auto& update{updates.EmplaceBack()};
  update.binding_index = binding_index;
  update.descriptors = descriptors;
  update.descriptor_count = descriptor_count;
}

void AddFieldUpdate(frame::FrameArray<ShaderBufferFieldUpdate>& updates,
                    ShaderBindingIndex binding_index,
                    ShaderFieldIndex field_index, const void* data,
                    usize size) {
  auto& update{updates.EmplaceBack()};
  update.binding_index = binding_index;
  update.field_index = field_index;
  update.data = data;
  update.size = size;
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet