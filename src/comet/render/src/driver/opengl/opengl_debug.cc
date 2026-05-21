// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/opengl/opengl_debug.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace rendering {
namespace gl {
namespace debug {
#ifdef COMET_RENDERING_USE_DEBUG_LABELS
namespace internal {
void SetDebugLabel(GLenum identifier, GLuint name, const schar* label) {
  if (label == nullptr || name == 0) {
    return;
  }

  glObjectLabel(identifier, name, -1, label);
}
}  // namespace internal

void SetStorageDebugLabel(GlNativeStorageHandle native_handle,
                          const schar* label) {
  internal::SetDebugLabel(GL_BUFFER, native_handle, label);
}

void SetUniformBufferDebugLabel(GlNativeUniformBufferHandle native_handle,
                                const schar* label) {
  internal::SetDebugLabel(GL_BUFFER, native_handle, label);
}

void SetTextureDebugLabel(GlNativeTextureHandle native_handle,
                          const schar* label) {
  internal::SetDebugLabel(GL_TEXTURE, native_handle, label);
}

void SetVertexAttributeDebugLabel(GlNativeVertexAttributeHandle native_handle,
                                  const schar* label) {
  internal::SetDebugLabel(GL_VERTEX_ARRAY, native_handle, label);
}

void SetProgramDebugLabel(GlNativeProgramHandle native_handle,
                          const schar* label) {
  internal::SetDebugLabel(GL_PROGRAM, native_handle, label);
}
#endif  // COMET_RENDERING_USE_DEBUG_LABELS
}  // namespace debug
}  // namespace gl
}  // namespace rendering
}  // namespace comet