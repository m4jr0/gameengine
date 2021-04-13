// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_VERTEX_BUFFER_H_
#define COMET_COMET_RENDERING_VERTEX_BUFFER_H_

#include "comet_precompile.h"

namespace comet {
namespace rendering {
class VertexBuffer {
 public:
  VertexBuffer() = default;
  VertexBuffer(const VertexBuffer&) = delete;
  VertexBuffer(VertexBuffer&&) noexcept = default;
  VertexBuffer& operator=(const VertexBuffer&) = delete;
  VertexBuffer& operator=(VertexBuffer&&) noexcept = default;
  virtual ~VertexBuffer() = default;

  static std::shared_ptr<VertexBuffer> Create(std::size_t, float* = nullptr);

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;
  virtual void SetData(std::size_t, const void*) = 0;
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_VERTEX_BUFFER_H_
