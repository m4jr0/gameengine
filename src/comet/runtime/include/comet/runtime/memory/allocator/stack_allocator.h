// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_MEMORY_ALLOCATOR_STACK_ALLOCATOR_H_
#define COMET_RUNTIME_MEMORY_ALLOCATOR_STACK_ALLOCATOR_H_

#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/core/concurrency/provider/thread_provider.h"
#include "comet/core/concurrency/thread/thread_common.h"
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

  void* AllocateAligned(usize size, Alignment align) override;
  void Deallocate(void*) override;

  // These functions are not thread-safe and must only be called during specific
  // synchronization points.
  void Clear();

 protected:
  void OnInitialize() override;
  void OnDestroy() override;

 private:
  using StackAllocatorMarker = u8*;

  MemoryTag memory_tag_{kEngineMemoryTagUntagged};
  usize capacity_{0};
  u8* root_{nullptr};
  StackAllocatorMarker marker_{nullptr};

#ifdef COMET_DEBUG_STACK_ALLOCATOR
  usize allocation_count_{0};
  usize clear_count_{0};
  usize peak_used_size_{0};
#endif  // COMET_DEBUG_STACK_ALLOCATOR
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

  void* AllocateAligned(usize size, Alignment align) override;
  void Deallocate(void*) override;

  // This function is not thread-safe and must only be called during specific
  // synchronization points.
  void Clear();

 protected:
  void OnInitialize() override;
  void OnDestroy() override;

 private:
  using FiberStackAllocatorMarker = u8*;

  COMET_DISABLE_PADDING_WARNING_BEGIN

  struct alignas(thread::kCacheLineSize) ThreadContext {
    u8* root{nullptr};
    FiberStackAllocatorMarker marker{nullptr};
  };

  COMET_DISABLE_PADDING_WARNING_END

  using ThreadContexts = thread::FiberThreadProvider<ThreadContext>;

  void AllocateCommonMemory();
  void ExtendCommonMemory(usize capacity);

  MemoryTag memory_tag_{kEngineMemoryTagUntagged};
  MemoryTag extended_memory_tag_{kEngineMemoryTagInvalid};
  usize base_capacity_{0};
  usize extended_capacity_{0};
  usize thread_capacity_{0};
  ThreadContexts thread_contexts_{};
  mutable fiber::FiberMutex mutex_{};
  u8* root_{nullptr};
  FiberStackAllocatorMarker marker_{nullptr};
  u8* extended_root_{nullptr};
  FiberStackAllocatorMarker extended_marker_{nullptr};

#ifdef COMET_DEBUG_STACK_ALLOCATOR
  usize thread_allocation_count_{0};
  usize common_allocation_count_{0};
  usize extended_allocation_count_{0};
  usize extended_grow_count_{0};
  usize clear_count_{0};
  usize peak_common_used_size_{0};
  usize peak_extended_used_size_{0};
  usize total_extended_capacity_{0};
  usize peak_extended_capacity_{0};
#endif  // COMET_DEBUG_STACK_ALLOCATOR
};

class IOStackAllocator : public StatefulAllocator {
 public:
  IOStackAllocator() = delete;
  IOStackAllocator(usize thread_capacity, MemoryTag memory_tag);
  IOStackAllocator(const IOStackAllocator&) = delete;
  IOStackAllocator(IOStackAllocator&&) = delete;
  IOStackAllocator& operator=(const IOStackAllocator&) = delete;
  IOStackAllocator& operator=(IOStackAllocator&&) = delete;

  void* AllocateAligned(usize size, Alignment align) override;
  void Deallocate(void*) override;

  // This function is not thread-safe and must only be called during specific
  // synchronization points.
  void Clear();

 protected:
  void OnInitialize() override;
  void OnDestroy() override;

 private:
  using IOStackAllocatorMarker = u8*;

  COMET_DISABLE_PADDING_WARNING_BEGIN

  struct alignas(thread::kCacheLineSize) ThreadContext {
    u8* root{nullptr};
    IOStackAllocatorMarker marker{nullptr};
  };

  COMET_DISABLE_PADDING_WARNING_END

  using ThreadContexts = thread::IOThreadProvider<ThreadContext>;

  MemoryTag memory_tag_{kEngineMemoryTagUntagged};
  usize thread_capacity_{0};
  ThreadContexts thread_contexts_{};

#ifdef COMET_DEBUG_STACK_ALLOCATOR
  usize allocation_count_{0};
  usize clear_count_{0};
#endif  // COMET_DEBUG_STACK_ALLOCATOR
};

class LockFreeStackAllocator : public StatefulAllocator {
 public:
  LockFreeStackAllocator() = delete;
  LockFreeStackAllocator(usize capacity, MemoryTag memory_tag);
  LockFreeStackAllocator(const LockFreeStackAllocator&) = delete;
  LockFreeStackAllocator(LockFreeStackAllocator&&) = delete;
  LockFreeStackAllocator& operator=(const LockFreeStackAllocator&) = delete;
  LockFreeStackAllocator& operator=(LockFreeStackAllocator&&) = delete;

  void* AllocateAligned(usize size, Alignment align) override;
  void Deallocate(void*) override;

  // This function is not thread-safe and must only be called during specific
  // synchronization points.
  void Clear();

 protected:
  void OnInitialize() override;
  void OnDestroy() override;

 private:
  using LockFreeStackAllocatorOffset = sptrdiff;
  static inline constexpr LockFreeStackAllocatorOffset kInvalidOffset_{-1};

  MemoryTag memory_tag_{kEngineMemoryTagUntagged};
  usize capacity_{0};

  static_assert(
      std::atomic<LockFreeStackAllocatorOffset>::is_always_lock_free,
      "std::atomic<LockFreeStackAllocatorOffset> must be always lock-free");
  std::atomic<LockFreeStackAllocatorOffset> offset_{kInvalidOffset_};

  u8* root_{nullptr};

#ifdef COMET_DEBUG_STACK_ALLOCATOR
  static_assert(std::atomic<usize>::is_always_lock_free,
                "std::atomic<usize> must be always lock-free");
  std::atomic<usize> allocation_count_{0};
  std::atomic<usize> peak_used_size_{0};
  std::atomic<usize> clear_count_{0};
#endif  // COMET_DEBUG_STACK_ALLOCATOR
};

template <typename Stack>
class DoubleStackAllocator : public StatefulAllocator {
 public:
  DoubleStackAllocator(usize stack_capacity, MemoryTag memory_tag)
      : stacks_{Stack{stack_capacity, memory_tag},
                Stack{stack_capacity, memory_tag}} {}

  DoubleStackAllocator(const DoubleStackAllocator&) = delete;
  DoubleStackAllocator(DoubleStackAllocator&&) = delete;
  DoubleStackAllocator& operator=(const DoubleStackAllocator&) = delete;
  DoubleStackAllocator& operator=(DoubleStackAllocator&&) = delete;
  ~DoubleStackAllocator() override = default;

  void* AllocateAligned(usize size, Alignment align) override {
    return stacks_[current_stack_].AllocateAligned(size, align);
  }

  void Deallocate(void* ptr) override {
    stacks_[current_stack_].Deallocate(ptr);
  }

  void SwapStacks() { current_stack_ = static_cast<u8>(!current_stack_); }

  void ClearCurrent() { stacks_[current_stack_].Clear(); }

 protected:
  void OnInitialize() override {
    stacks_[0].Initialize();
    stacks_[1].Initialize();
  }

  void OnDestroy() override {
    stacks_[0].Destroy();
    stacks_[1].Destroy();
  }

 private:
  u8 current_stack_{0};
  Stack stacks_[2];
};

template <typename Stack>
class FiberDoubleStackAllocator : public StatefulAllocator {
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
  ~FiberDoubleStackAllocator() override = default;

  void* AllocateAligned(usize size, Alignment align) override {
    return stacks_[current_stack_].AllocateAligned(size, align);
  }

  void Deallocate(void* ptr) override {
    stacks_[current_stack_].Deallocate(ptr);
  }

  void SwapStacks() { current_stack_ = static_cast<u8>(!current_stack_); }

  void ClearCurrent() { stacks_[current_stack_].Clear(); }

 protected:
  void OnInitialize() override {
    stacks_[0].Initialize();
    stacks_[1].Initialize();
  }

  void OnDestroy() override {
    stacks_[0].Destroy();
    stacks_[1].Destroy();
  }

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
  ~StaticStackAllocator() override = default;

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
  COMET_ASSERT(size > 0, "StaticStackAllocator::AllocateAligned",
               "allocation size is zero");

  auto* p{AlignPointer(marker_, align)};

  COMET_ASSERT(
      p + size <= root_ + Capacity, "StaticStackAllocator::AllocateAligned",
      "allocation exceeds capacity", "size", size, "capacity", Capacity);

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

#endif  // COMET_RUNTIME_MEMORY_ALLOCATOR_STACK_ALLOCATOR_H_
