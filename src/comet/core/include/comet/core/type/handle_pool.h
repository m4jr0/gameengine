// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_TYPE_HANDLE_POOL_H_
#define COMET_CORE_TYPE_HANDLE_POOL_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type/gid_pool.h"
#include "comet/core/type/handle.h"

namespace comet {
template <typename Tag>
class HandlePool {
 public:
  using H = Handle<Tag>;

  HandlePool() = default;
  explicit HandlePool(memory::Allocator* allocator) : gid_pool_{allocator} {}

  HandlePool(const HandlePool&) = delete;
  HandlePool(HandlePool&&) noexcept = default;
  HandlePool& operator=(const HandlePool&) = delete;
  HandlePool& operator=(HandlePool&&) noexcept = default;
  ~HandlePool() = default;

  void Initialize(memory::Allocator* allocator) {
    gid_pool_.Initialize(allocator);
  }

  H Generate() noexcept { return H{gid_pool_.Generate()}; }

  bool IsAlive(H handle) const noexcept {
    return handle.IsValid() && gid_pool_.IsAlive(handle.GetValue());
  }

  void Destroy(H handle) noexcept {
    if (!IsAlive(handle)) {
      return;
    }

    gid_pool_.Destroy(handle.GetValue());
  }

  void Shutdown() noexcept { gid_pool_.Shutdown(); }

 private:
  gid::GidPool gid_pool_{};
};
}  // namespace comet

#endif  // COMET_CORE_TYPE_HANDLE_POOL_H_