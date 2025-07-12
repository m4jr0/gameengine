// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MEMORY_ALLOCATOR_STACK_ALLOCATOR_H_
#define COMET_COMET_CORE_MEMORY_ALLOCATOR_STACK_ALLOCATOR_H_

#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/core/concurrency/provider/thread_provider.h"
#include "comet/core/concurrency/provider/thread_provider_manager.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/stateful_allocator.h"
#include "comet/core/memory/memory.h"

namespace comet {
namespace memory {
class StackAllocator : public StatefulAllocator {
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

  bool is_destroyed_{false};
  MemoryTag memory_tag_{kEngineMemoryTagUntagged};
  usize capacity_{0};
  u8* root_{nullptr};
  StackAllocatorMarker marker_{nullptr};
};

class FiberStackAllocator : public StatefulAllocator {
 public:
  FiberStackAllocator() = delete;
  FiberStackAllocator(usize base_capacity, MemoryTag memory_tag,
                      MemoryTag extended_memory_tag = kEngineMemoryTagInvalid);
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
  void ExtendCommonMemory(usize capacity);

  MemoryTag memory_tag_{kEngineMemoryTagUntagged};
  MemoryTag extended_memory_tag_{kEngineMemoryTagInvalid};
  usize base_capacity_{0};
  usize extended_capacity_{0};
  usize thread_capacity_{0};
  ThreadContexts thread_contexts_{thread::ThreadProviderManager::Get()
                                      .AllocateFiberProvider<ThreadContext>()};
  mutable fiber::FiberMutex mutex_{};
  u8* root_{nullptr};
  FiberStackAllocatorMarker marker_{nullptr};
  u8* extended_root_{nullptr};
  FiberStackAllocatorMarker extended_marker_{nullptr};
};

class IOStackAllocator : public StatefulAllocator {
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

  bool is_destroyed_{false};
  MemoryTag memory_tag_{kEngineMemoryTagUntagged};
  usize thread_capacity_{0};
  ThreadContexts thread_contexts_{
      thread::ThreadProviderManager::Get().AllocateIOProvider<ThreadContext>()};
};

class LockFreeStackAllocator : public StatefulAllocator {
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
class DoubleStackAllocator : public memory::StatefulAllocator {
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
    StatefulAllocator::Initialize();
    stacks_[0].Initialize();
    stacks_[1].Initialize();
  }

  void Destroy() override {
    StatefulAllocator::Destroy();
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

template <typename Stack>
class FiberDoubleStackAllocator : public memory::StatefulAllocator {
 public:
  FiberDoubleStackAllocator(
      usize stack_capacity, MemoryTag memory_tag,
      MemoryTag extended_memory_tag_1 = kEngineMemoryTagInvalid,
      MemoryTag extended_memory_tag_2 = kEngineMemoryTagInvalid)
      : stacks_{Stack{stack_capacity, memory_tag, extended_memory_tag_1},
                Stack{stack_capacity, memory_tag, extended_memory_tag_2}} {}

  FiberDoubleStackAllocator(const FiberDoubleStackAllocator&) = delete;
  FiberDoubleStackAllocator(FiberDoubleStackAllocator&&) = delete;
  FiberDoubleStackAllocator& operator=(const FiberDoubleStackAllocator&) =
      delete;
  FiberDoubleStackAllocator& operator=(FiberDoubleStackAllocator&&) = delete;
  ~FiberDoubleStackAllocator() = default;

  void Initialize() override {
    StatefulAllocator::Initialize();
    stacks_[0].Initialize();
    stacks_[1].Initialize();
  }

  void Destroy() override {
    StatefulAllocator::Destroy();
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

template <usize Capacity, Alignment Align = kTrivialTypeMaxAlignment>
class StaticStackAllocator : public Allocator {
 public:
  StaticStackAllocator() = default;
  StaticStackAllocator(const StaticStackAllocator&) = delete;
  StaticStackAllocator(StaticStackAllocator&&) = delete;
  StaticStackAllocator& operator=(const StaticStackAllocator&) = delete;
  StaticStackAllocator& operator=(StaticStackAllocator&&) = delete;
  virtual ~StaticStackAllocator() = default;

  void* AllocateAligned(usize size, Alignment align) override;
  void Deallocate(void*) override;
  void Clear();

 private:
  using StackAllocatorMarker = u8*;

  alignas(Align) u8 root_[Capacity]{};
  StackAllocatorMarker marker_{root_};
};

template <usize Capacity, Alignment Align>
inline void* StaticStackAllocator<Capacity, Align>::AllocateAligned(
    usize size, Alignment align) {
  COMET_ASSERT(size > 0, "Allocation size provided is 0!");
  auto* p{AlignPointer(marker_, align)};
  COMET_ASSERT(p + size <= root_ + Capacity,
               "Could not allocate enough memory (", size, ")!");
  marker_ = p + size;
  return p;
}

template <usize Capacity, Alignment Align>
inline void StaticStackAllocator<Capacity, Align>::Deallocate(void*) {
  // A stack allocator does not support individual deallocations, as it is
  // intended for temporary data only. Memory is only released when Clear() is
  // called, which resets the entire stack.
}

template <usize Capacity, Alignment Align>
inline void StaticStackAllocator<Capacity, Align>::Clear() {
  marker_ = root_;
}
}  // namespace memory
}  // namespace comet

#endif  // COMET_COMET_CORE_MEMORY_ALLOCATOR_STACK_ALLOCATOR_H_
