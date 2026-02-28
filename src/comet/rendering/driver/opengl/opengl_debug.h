// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_OPENGL_DEBUG_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_OPENGL_DEBUG_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/rendering/driver/opengl/data/opengl_shader.h"
#include "comet/rendering/driver/opengl/data/opengl_shader_module.h"
#include "comet/rendering/driver/opengl/data/opengl_storage.h"
#include "comet/rendering/driver/opengl/data/opengl_texture.h"

namespace comet {
namespace rendering {
namespace gl {
namespace debug {
#ifdef COMET_RENDERING_USE_DEBUG_LABELS
namespace internal {
void SetDebugLabel(GLenum identifier, GLuint name, const schar* label);
}  // namespace internal

void SetStorageDebugLabel(StorageHandle handle, const schar* label);
void SetUniformBufferDebugLabel(UniformBufferHandle handle, const schar* label);
void SetTextureDebugLabel(TextureHandle handle, const schar* label);
void SetVertexAttributeDebugLabel(VertexAttributeHandle handle,
                                  const schar* label);
void SetShaderModuleDebugLabel(ShaderModuleHandle handle, const schar* label);
void SetShaderDebugLabel(ShaderHandle handle, const schar* label);
#endif  // COMET_RENDERING_USE_DEBUG_LABELS
}  // namespace debug
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#ifdef COMET_RENDERING_USE_DEBUG_LABELS
#define COMET_GL_SET_STORAGE_DEBUG_LABEL(handle, label) \
  comet::rendering::gl::debug::SetStorageDebugLabel(handle, label)
#define COMET_GL_SET_UNIFORM_BUFFER_DEBUG_LABEL(handle, label) \
  comet::rendering::gl::debug::SetUniformBufferDebugLabel(handle, label);
#define COMET_GL_SET_TEXTURE_DEBUG_LABEL(handle, label) \
  comet::rendering::gl::debug::SetTextureDebugLabel(handle, label);
#define COMET_GL_SET_VERTEX_ATTRIBUTE_DEBUG_LABEL(handle, label) \
  comet::rendering::gl::debug::SetVertexAttributeDebugLabel(handle, label);
#define COMET_GL_SET_SHADER_MODULE_DEBUG_LABEL(handle, label) \
  comet::rendering::gl::debug::SetShaderModuleDebugLabel(handle, label);
#define COMET_GL_SET_SHADER_DEBUG_LABEL(handle, label) \
  comet::rendering::gl::debug::SetShaderDebugLabel(handle, label);
#else
#define COMET_GL_SET_STORAGE_DEBUG_LABEL(handle, label)
#define COMET_GL_SET_UNIFORM_BUFFER_DEBUG_LABEL(handle, label)
#define COMET_GL_SET_TEXTURE_DEBUG_LABEL(handle, label)
#define COMET_GL_SET_VERTEX_ATTRIBUTE_DEBUG_LABEL(handle, label)
#define COMET_GL_SET_SHADER_MODULE_DEBUG_LABEL(handle, label)
#define COMET_GL_SET_SHADER_DEBUG_LABEL(handle, label)
#endif  // COMET_RENDERING_USE_DEBUG_LABELS

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_OPENGL_DEBUG_H_
