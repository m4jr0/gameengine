// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_ID_GID_POOL_H_
#define COMET_CORE_ID_GID_POOL_H_

#include "comet/core/container/array.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/id/gid.h"

namespace comet {
namespace gid {
class GidPool {
 public:
  GidPool() = default;
  explicit GidPool(memory::Allocator* allocator);
  GidPool(const GidPool&) = delete;
  GidPool(GidPool&& other) noexcept;
  GidPool& operator=(const GidPool&) = delete;
  GidPool& operator=(GidPool&& other) noexcept;
  ~GidPool() = default;

  void Initialize(memory::Allocator* allocator);
  void Shutdown();

  Gid Generate();
  void Destroy(Gid id);

  bool IsAlive(Gid id) const;

 private:
  memory::Allocator* allocator_{nullptr};
  Array<IdGeneration> generations_{};
  Array<Gid> free_ids_{};
};
}  // namespace gid
}  // namespace comet

#endif  // COMET_CORE_ID_GID_POOL_H_