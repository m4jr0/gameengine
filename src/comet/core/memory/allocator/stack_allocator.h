// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MEMORY_ALLOCATOR_STACK_ALLOCATOR_H_
#define COMET_COMET_CORE_MEMORY_ALLOCATOR_STACK_ALLOCATOR_H_

#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/core/concurrency/provider/thread_provider.h"
#include "comet/core/concurrency/provider/thread_provider_manager.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/aligned_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/fixed_size_array.h"

namespace comet {
namespace memory {
class StackAllocator : public AlignedAllocator {
 public:
  StackAllocator() = default;
  StackAllocator(usize capacity, MemoryTag memory_tag);
  StackAllocator(const StackAllocator&) = delete;
  StackAllocator(StackAllocator&& other) noexcept;
  StackAllocator& operator=(const StackAllocator&) = delete;
  StackAllocator& operator=(StackAllocator&& other) noexcept;

  void Initialize() override;
  void Destroy() override;

  void* AllocateAligned(usize size, Alignment align) override;
  void Deallocate(void*) override;

  // These functions are not thread-safe and must only be called during specific
  // synchronization points.
  void Clear();

 private:
  using StackAllocatorMarker = u8*;

  MemoryTag memory_tag_{kEngineMemoryTagUntagged};
  usize capacity_{0};
  u8* root_{nullptr};
  StackAllocatorMarker marker_{nullptr};
};

class FiberStackAllocator : public AlignedAllocator {
 public:
  FiberStackAllocator() = delete;
  FiberStackAllocator(usize base_capacity, MemoryTag memory_tag);
  FiberStackAllocator(const FiberStackAllocator&) = delete;
  FiberStackAllocator(FiberStackAllocator&&) = delete;
  FiberStackAllocator& operator=(const FiberStackAllocator&) = delete;
  FiberStackAllocator& operator=(FiberStackAllocator&&) = delete;

  void Initialize() override;
  void Destroy() override;

  void* AllocateAligned(usize size, Alignment align) override;
  void Deallocate(void*) override;

  // This function is not thread-safe and must only be called during specific
  // synchronization points.
  void Clear();

 private:
  using FiberStackAllocatorMarker = u8*;

  struct ThreadContext {
    u8* root{nullptr};
    FiberStackAllocatorMarker marker{nullptr};
  } typedef ThreadContext;

  using ThreadContexts = thread::FiberThreadProvider<ThreadContext>;

  void AllocateCommonMemory();

  MemoryTag memory_tag_{kEngineMemoryTagUntagged};
  usize base_capacity_{0};
  usize current_capacity_{0};
  usize thread_capacity_{0};
  ThreadContexts thread_contexts_{thread::ThreadProviderManager::Get()
                                      .AllocateFiberProvider<ThreadContext>()};
  mutable fiber::FiberMutex mutex_{};
  u8* root_{nullptr};
  FiberStackAllocatorMarker marker_{nullptr};
};

class IOStackAllocator : public AlignedAllocator {
 public:
  IOStackAllocator() = delete;
  IOStackAllocator(usize thread_capacity, MemoryTag memory_tag);
  IOStackAllocator(const IOStackAllocator&) = delete;
  IOStackAllocator(IOStackAllocator&&) = delete;
  IOStackAllocator& operator=(const IOStackAllocator&) = delete;
  IOStackAllocator& operator=(IOStackAllocator&&) = delete;

  void Initialize() override;
  void Destroy() override;

  void* AllocateAligned(usize size, Alignment align) override;
  void Deallocate(void*) override;

  // This function is not thread-safe and must only be called during specific
  // synchronization points.
  void Clear();

 private:
  using IOStackAllocatorMarker = u8*;

  struct ThreadContext {
    u8* root{nullptr};
    IOStackAllocatorMarker marker{nullptr};
  } typedef ThreadContext;

  using ThreadContexts = thread::IOThreadProvider<ThreadContext>;

  MemoryTag memory_tag_{kEngineMemoryTagUntagged};
  usize thread_capacity_{0};
  ThreadContexts thread_contexts_{
      thread::ThreadProviderManager::Get().AllocateIOProvider<ThreadContext>()};
};

class LockFreeStackAllocator : public AlignedAllocator {
 public:
  LockFreeStackAllocator() = delete;
  LockFreeStackAllocator(usize capacity, MemoryTag memory_tag);
  LockFreeStackAllocator(const LockFreeStackAllocator&) = delete;
  LockFreeStackAllocator(LockFreeStackAllocator&&) = delete;
  LockFreeStackAllocator& operator=(const LockFreeStackAllocator&) = delete;
  LockFreeStackAllocator& operator=(LockFreeStackAllocator&&) = delete;

  void Initialize() override;
  void Destroy() override;

  void* AllocateAligned(usize size, Alignment align) override;
  void Deallocate(void*) override;

  // This function is not thread-safe and must only be called during specific
  // synchronization points.
  void Clear();

 private:
  using LockFreeStackAllocatorOffset = sptrdiff;
  static inline constexpr LockFreeStackAllocatorOffset kInvalidOffset_{-1};

  MemoryTag memory_tag_{kEngineMemoryTagUntagged};
  usize capacity_{0};
  static_assert(std::atomic<LockFreeStackAllocatorOffset>::is_always_lock_free,
                "std::atomic<LockFreeStackAllocatorOffset> needs to be always "
                "lock-free. Unsupported "
                "architecture");
  std::atomic<LockFreeStackAllocatorOffset> offset_{kInvalidOffset_};
  u8* root_{nullptr};
};

template <typename Stack>
class DoubleStackAllocator : public memory::AlignedAllocator {
 public:
  DoubleStackAllocator(usize stack_capacity, MemoryTag memory_tag)
      : stacks_{Stack{stack_capacity, memory_tag},
                Stack{stack_capacity, memory_tag}} {}

  DoubleStackAllocator(const DoubleStackAllocator&) = delete;
  DoubleStackAllocator(DoubleStackAllocator&&) = delete;
  DoubleStackAllocator& operator=(const DoubleStackAllocator&) = delete;
  DoubleStackAllocator& operator=(DoubleStackAllocator&&) = delete;
  ~DoubleStackAllocator() = default;

  void Initialize() override {
    AlignedAllocator::Initialize();
    stacks_[0].Initialize();
    stacks_[1].Initialize();
  }

  void Destroy() override {
    AlignedAllocator::Destroy();
    stacks_[0].Destroy();
    stacks_[1].Destroy();
  }

  void* AllocateAligned(usize size, memory::Alignment align) override {
    return stacks_[current_stack_].AllocateAligned(size, align);
  }

  void Deallocate(void* ptr) override {
    stacks_[current_stack_].Deallocate(ptr);
  }

  void SwapStacks() { current_stack_ = static_cast<u8>(!current_stack_); }

  void ClearCurrent() { stacks_[current_stack_].Clear(); }

 private:
  u8 current_stack_{0};
  Stack stacks_[2];
};
}  // namespace memory
}  // namespace comet

#endif  // COMET_COMET_CORE_MEMORY_ALLOCATOR_STACK_ALLOCATOR_H_
