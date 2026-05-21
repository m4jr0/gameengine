// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_MEMORY_ALLOCATOR_STATEFUL_ALLOCATOR_H_
#define COMET_CORE_MEMORY_ALLOCATOR_STATEFUL_ALLOCATOR_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"

namespace comet {
namespace memory {
class StatefulAllocator : public Allocator {
 public:
  ~StatefulAllocator() override;

  void Initialize();
  void Destroy();

  bool IsInitialized() const noexcept;

 protected:
  virtual void OnInitialize();
  virtual void OnDestroy();

 private:
  bool is_initialized_{false};
};
}  // namespace memory
}  // namespace comet

#endif  // COMET_CORE_MEMORY_ALLOCATOR_STATEFUL_ALLOCATOR_H_
