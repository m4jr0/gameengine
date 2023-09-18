// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "memory_manager.h"

#include "allocation_tracking.h"

namespace comet {
namespace memory {
MemoryManager& MemoryManager::Get() {
  static MemoryManager singleton{};
  return singleton;
}

void MemoryManager::Initialize() {
  Manager::Initialize();
  one_frame_allocator_.Initialize();
  two_frame_allocator_.Initialize();
  tstring_allocator_.Initialize();
}

void MemoryManager::Shutdown() {
  one_frame_allocator_.Destroy();
  two_frame_allocator_.Destroy();
  tstring_allocator_.Destroy();
  Manager::Shutdown();
}

void MemoryManager::Update() {
  one_frame_allocator_.Clear();
  two_frame_allocator_.SwapFrames();
  two_frame_allocator_.ClearCurrent();
}

uindex MemoryManager::GetAllocatedMemory() const { return GetMemoryUse(); }

OneFrameAllocator& MemoryManager::GetOneFrameAllocator() {
  return one_frame_allocator_;
}

TwoFrameAllocator& MemoryManager::GetTwoFrameAllocator() {
  return two_frame_allocator_;
}

TStringAllocator& MemoryManager::GetTStringAllocator() {
  return tstring_allocator_;
}
}  // namespace memory
}  // namespace comet
