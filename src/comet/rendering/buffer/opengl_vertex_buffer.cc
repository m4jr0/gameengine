// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "opengl_vertex_buffer.h"

#include "glad/glad.h"

namespace comet {
namespace rendering {
OpenGlVertexBuffer::OpenGlVertexBuffer(std::size_t size,
                                       const float* vertices) {
  glCreateBuffers(1, &vertex_buffer_);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_DYNAMIC_DRAW);
}

OpenGlVertexBuffer::OpenGlVertexBuffer(OpenGlVertexBuffer&& other) noexcept
    : VertexBuffer(std::move(other)),
      vertex_buffer_(std::move(other.vertex_buffer_)) {}

OpenGlVertexBuffer& OpenGlVertexBuffer::operator=(
    OpenGlVertexBuffer&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  VertexBuffer::operator=(std::move(other));
  vertex_buffer_ = std::move(other.vertex_buffer_);
  return *this;
}

OpenGlVertexBuffer::~OpenGlVertexBuffer() {
  glDeleteBuffers(1, &vertex_buffer_);
}

void OpenGlVertexBuffer::Bind() const {
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
}

void OpenGlVertexBuffer::Unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }

void OpenGlVertexBuffer::SetData(std::size_t size, const void* data) {
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}
}  // namespace rendering
}  // namespace comet
