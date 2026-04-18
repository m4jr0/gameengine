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

void SetStorageDebugLabel(GlNativeStorageHandle native_handle,
                          const schar* label);
void SetUniformBufferDebugLabel(GlNativeUniformBufferHandle native_handle,
                                const schar* label);
void SetTextureDebugLabel(GlNativeTextureHandle native_handle,
                          const schar* label);
void SetVertexAttributeDebugLabel(GlNativeVertexAttributeHandle native_handle,
                                  const schar* label);
void SetProgramDebugLabel(GlNativeProgramHandle native_handle,
                          const schar* label);
#endif  // COMET_RENDERING_USE_DEBUG_LABELS
}  // namespace debug
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#ifdef COMET_RENDERING_USE_DEBUG_LABELS
#define COMET_GL_SET_STORAGE_DEBUG_LABEL(native_handle, label) \
  comet::rendering::gl::debug::SetStorageDebugLabel(native_handle, label)
#define COMET_GL_SET_UNIFORM_BUFFER_DEBUG_LABEL(native_handle, label) \
  comet::rendering::gl::debug::SetUniformBufferDebugLabel(native_handle, label)
#define COMET_GL_SET_TEXTURE_DEBUG_LABEL(native_handle, label) \
  comet::rendering::gl::debug::SetTextureDebugLabel(native_handle, label)
#define COMET_GL_SET_VERTEX_ATTRIBUTE_DEBUG_LABEL(native_handle, label)    \
  comet::rendering::gl::debug::SetVertexAttributeDebugLabel(native_handle, \
                                                            label)
#define COMET_GL_SET_PROGRAM_DEBUG_LABEL(native_handle, label) \
  comet::rendering::gl::debug::SetProgramDebugLabel(native_handle, label)
#else
#define COMET_GL_SET_STORAGE_DEBUG_LABEL(native_handle, label)
#define COMET_GL_SET_UNIFORM_BUFFER_DEBUG_LABEL(native_handle, label)
#define COMET_GL_SET_TEXTURE_DEBUG_LABEL(native_handle, label)
#define COMET_GL_SET_VERTEX_ATTRIBUTE_DEBUG_LABEL(native_handle, label)
#define COMET_GL_SET_PROGRAM_DEBUG_LABEL(native_handle, label)
#endif  // COMET_RENDERING_USE_DEBUG_LABELS

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_OPENGL_DEBUG_H_