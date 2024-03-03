// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MEMORY_MEMORY_MANAGER_H_
#define COMET_COMET_CORE_MEMORY_MEMORY_MANAGER_H_

#include "comet_precompile.h"

#include "comet/core/conf/configuration_manager.h"
#include "comet/core/manager.h"
#include "comet/core/memory/allocator/frame_allocator.h"
#include "comet/core/memory/allocator/tstring_allocator.h"
#include "comet/core/memory/memory.h"

namespace comet {
namespace memory {
class MemoryManager : public Manager {
 public:
  static MemoryManager& Get();

  MemoryManager() = default;
  MemoryManager(const MemoryManager&) = delete;
  MemoryManager(MemoryManager&&) = delete;
  MemoryManager& operator=(const MemoryManager&) = delete;
  MemoryManager& operator=(MemoryManager&&) = delete;
  virtual ~MemoryManager() = default;

  void Initialize() override;
  void Shutdown() override;
  void Update();

  uindex GetAllocatedMemory() const;
  OneFrameAllocator& GetOneFrameAllocator();
  TwoFrameAllocator& GetTwoFrameAllocator();
  TStringAllocator& GetTStringAllocator();

 private:
  OneFrameAllocator one_frame_allocator_{
      COMET_CONF_U32(conf::kCoreOneFrameAllocatorCapacity),
      MemoryTag::OneFrame};
  TwoFrameAllocator two_frame_allocator_{
      COMET_CONF_U32(conf::kCoreTwoFrameAllocatorCapacity)};
  TStringAllocator tstring_allocator_{
      COMET_CONF_U32(conf::kCoreTStringAllocatorCapacity)};
};
}  // namespace memory
}  // namespace comet

#endif  // COMET_COMET_CORE_MEMORY_MEMORY_MANAGER_H_
