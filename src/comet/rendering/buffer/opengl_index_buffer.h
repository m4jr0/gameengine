// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_OPENGL_INDEX_BUFFER_H_
#define COMET_COMET_RENDERING_OPENGL_INDEX_BUFFER_H_

#include "comet_precompile.h"
#include "index_buffer.h"

namespace comet {
namespace rendering {
class OpenGlIndexBuffer : public IndexBuffer {
 public:
  OpenGlIndexBuffer(std::size_t, unsigned int*);
  OpenGlIndexBuffer(const OpenGlIndexBuffer&) = delete;
  OpenGlIndexBuffer(OpenGlIndexBuffer&&) noexcept;
  OpenGlIndexBuffer& operator=(const OpenGlIndexBuffer&) = delete;
  OpenGlIndexBuffer& operator=(OpenGlIndexBuffer&&) noexcept;
  virtual ~OpenGlIndexBuffer();

  virtual void Bind() const override;
  virtual void Unbind() const override;
  std::size_t GetSize() const override;

 private:
  std::size_t size_;
  GLuint index_buffer_ = 0;
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_OPENGL_INDEX_BUFFER_H_
