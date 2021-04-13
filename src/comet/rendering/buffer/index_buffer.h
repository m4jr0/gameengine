// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_INDEX_BUFFER_H_
#define COMET_COMET_RENDERING_INDEX_BUFFER_H_

#include "comet_precompile.h"

namespace comet {
namespace rendering {
class IndexBuffer {
 public:
  IndexBuffer() = default;
  IndexBuffer(const IndexBuffer&) = delete;
  IndexBuffer(IndexBuffer&&) noexcept = default;
  IndexBuffer& operator=(const IndexBuffer&) = delete;
  IndexBuffer& operator=(IndexBuffer&&) noexcept = default;
  virtual ~IndexBuffer() = default;

  static std::shared_ptr<IndexBuffer> Create(std::size_t, unsigned int*);

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;
  virtual std::size_t GetSize() const = 0;
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_INDEX_BUFFER_H_
