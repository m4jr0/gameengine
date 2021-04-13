// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "opengl_index_buffer.h"

#include "glad/glad.h"

namespace comet {
namespace rendering {
OpenGlIndexBuffer::OpenGlIndexBuffer(std::size_t size, unsigned int* indices)
    : size_(size) {
  glCreateBuffers(1, &index_buffer_);
  glBindBuffer(GL_ARRAY_BUFFER, index_buffer_);
  glBufferData(GL_ARRAY_BUFFER, size * sizeof(unsigned int), indices,
               GL_STATIC_DRAW);
}

OpenGlIndexBuffer::OpenGlIndexBuffer(OpenGlIndexBuffer&& other) noexcept
    : IndexBuffer(std::move(other)),
      size_(std::move(other.size_)),
      index_buffer_(std::move(other.index_buffer_)) {}

OpenGlIndexBuffer& OpenGlIndexBuffer::operator=(
    OpenGlIndexBuffer&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  IndexBuffer::operator=(std::move(other));
  size_ = std::move(other.size_);
  index_buffer_ = std::move(other.index_buffer_);
  return *this;
}

OpenGlIndexBuffer::~OpenGlIndexBuffer() { glDeleteBuffers(1, &index_buffer_); }

void OpenGlIndexBuffer::Bind() const {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
}

void OpenGlIndexBuffer::Unbind() const {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

std::size_t OpenGlIndexBuffer::GetSize() const { return size_; }

}  // namespace rendering
}  // namespace comet
