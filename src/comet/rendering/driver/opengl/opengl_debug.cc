// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "opengl_debug.h"

namespace comet {
namespace rendering {
namespace gl {
namespace debug {
#ifdef COMET_RENDERING_USE_DEBUG_LABELS
namespace internal {
void SetDebugLabel(GLenum identifier, GLuint name, const schar* label) {
  glObjectLabel(identifier, name, -1, label);
}
}  // namespace internal
#endif

void SetStorageDebugLabel(StorageHandle handle, const schar* label) {
  internal::SetDebugLabel(GL_BUFFER, handle, label);
}

void SetUniformBufferDebugLabel(UniformBufferHandle handle,
                                const schar* label) {
  internal::SetDebugLabel(GL_BUFFER, handle, label);
}

void SetTextureDebugLabel(TextureHandle handle, const schar* label) {
  internal::SetDebugLabel(GL_BUFFER, handle, label);
}

void SetVertexAttributeDebugLabel(VertexAttributeHandle handle,
                                  const schar* label) {
  internal::SetDebugLabel(GL_VERTEX_ARRAY, handle, label);
}

void SetShaderModuleDebugLabel(ShaderModuleHandle handle, const schar* label) {
  internal::SetDebugLabel(GL_SHADER, handle, label);
}

void SetShaderDebugLabel(ShaderHandle handle, const schar* label) {
  internal::SetDebugLabel(GL_PROGRAM, handle, label);
}
// COMET_RENDERING_USE_DEBUG_LABELS
}  // namespace debug
}  // namespace gl
}  // namespace rendering
}  // namespace comet
