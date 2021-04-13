// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_OPENGL_VERTEX_BUFFER_H_
#define COMET_COMET_RENDERING_OPENGL_VERTEX_BUFFER_H_

#include "comet_precompile.h"
#include "vertex_buffer.h"

namespace comet {
namespace rendering {
class OpenGlVertexBuffer : public VertexBuffer {
 public:
  OpenGlVertexBuffer(std::size_t, const float* = nullptr);
  OpenGlVertexBuffer(const OpenGlVertexBuffer&) = delete;
  OpenGlVertexBuffer(OpenGlVertexBuffer&&) noexcept;
  OpenGlVertexBuffer& operator=(const OpenGlVertexBuffer&) = delete;
  OpenGlVertexBuffer& operator=(OpenGlVertexBuffer&&) noexcept;
  ~OpenGlVertexBuffer();

  void Bind() const override;
  void Unbind() const override;
  void SetData(std::size_t, const void*) override;

 private:
  GLuint vertex_buffer_ = 0;
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_OPENGL_VERTEX_BUFFER_H_
