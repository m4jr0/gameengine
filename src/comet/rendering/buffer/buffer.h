// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_BUFFER_BUFFER_H_
#define COMET_COMET_RENDERING_BUFFER_BUFFER_H_

#include "comet_precompile.h"

namespace comet {
namespace rendering {
class Buffer {
 public:
  Buffer() = default;
  Buffer(const Buffer&) = delete;
  Buffer(Buffer&&) noexcept = default;
  Buffer& operator=(const Buffer&) = delete;
  Buffer& operator=(Buffer&&) noexcept = default;
  virtual ~Buffer() = default;

  static std::shared_ptr<Buffer> Create(std::size_t size,
                                        float* vertices = nullptr);

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;
  virtual void SetData(std::size_t size, const void* data) = 0;
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_BUFFER_BUFFER_H_
