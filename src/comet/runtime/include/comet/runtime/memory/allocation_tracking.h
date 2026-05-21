// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_MEMORY_ALLOCATION_TRACKING_H_
#define COMET_RUNTIME_MEMORY_ALLOCATION_TRACKING_H_

#include "comet/core/define.h"

// TODO(m4jr0): Improve allocation tracking for MSVC and other compilers.
// Currently, tracking is partially functional on MSVC and not working properly
// on GCC. Tag allocation, however, appears to be functioning correctly in both
// cases.

#ifdef COMET_TRACK_ALLOCATIONS
// External. ///////////////////////////////////////////////////////////////////
#include <atomic>
#include <mutex>
#include <shared_mutex>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/map.h"

#ifdef COMET_MSVC
#include "comet/core/windows.h"
#else
#include <dlfcn.h>
#include <sys/mman.h>
#endif  // COMET_MSVC

namespace comet {
namespace memory {
struct MemoryTagUseSnapshot {
  memory::MemoryTag tag{memory::kEngineMemoryTagInvalid};
  usize size{0};
};

struct MemoryUseSnapshot {
  static constexpr usize kMaxTagCount{512};

  usize memory_use{0};
  usize tag_count{0};
  MemoryTagUseSnapshot tags[kMaxTagCount]{};
};

namespace internal {
// TODO(m4jr0): Consider using a lock-free solution (though it's primarily for
// debugging, so... yeah...).
struct AllocationInfo {
  usize size;
  MemoryTag tag;
};

// TODO(m4jr0): Consider using a lock-free solution (though it's primarily for
// debugging, so... yeah...).
struct TrackedAllocations {
  TrackedAllocations(memory::Allocator* allocator);
  ~TrackedAllocations();

  void Initialize();
  void Destroy();

  void Push(void* ptr, usize size);
  usize Pop(void* ptr);

 private:
  bool is_initialized{false};
  std::recursive_mutex mutex{};
  Map<void*, usize> allocations{};
  memory::Allocator* allocator{nullptr};
};

// TODO(m4jr0): Consider using a lock-free solution (though it's primarily for
// debugging, so... yeah...).
struct TrackedTags {
  TrackedTags(memory::Allocator* allocator);
  ~TrackedTags();

  void Initialize();
  void Destroy();

  void IncreasePlatform(void* ptr, usize size, MemoryTag memory_tag);
  void DecreasePlatform(void* ptr);
  void IncreaseTag(usize size, MemoryTag memory_tag);
  void DecreaseTag(usize size, MemoryTag memory_tag);
  void IncreaseTaggedHeapPool(usize size);
  void DecreaseTaggedHeapPool(usize size);
  void IncreaseTaggedHeap(usize size, MemoryTag memory_tag);
  void DecreaseTaggedHeap(MemoryTag memory_tag);
  // TODO(m4jr0): Consider using a thread-unsafe function.
  // It might be suitable at specific points to avoid locking overhead.
  Map<MemoryTag, usize> GetTagUse();

 private:
  bool is_initialized{false};
  std::shared_mutex platform_mutex{};
  std::shared_mutex tagged_heap_mutex{};
  Map<void*, AllocationInfo> platform_allocations{};
  Map<MemoryTag, usize> platform_tags{};
  Map<MemoryTag, usize> tagged_heap_tags{};
  memory::Allocator* allocator{nullptr};
};

struct MemoryUse {
  static MemoryUse& Get();

  ~MemoryUse();

  void Initialize();
  void Destroy();

  bool is_tracking{false};

  static_assert(std::atomic<usize>::is_always_lock_free,
                "std::atomic<usize> must be always lock-free");
  std::atomic<usize> total_allocated{0};
  std::atomic<usize> total_freed{0};

  memory::PlatformAllocator allocator{memory::kEngineMemoryTagDebug};
  TrackedAllocations allocations{&allocator};
  TrackedTags tags{&allocator};

 private:
  bool is_initialized{false};
};

class ScopedFlagToggle {
 public:
  explicit ScopedFlagToggle(bool& flag);
  ~ScopedFlagToggle();

 private:
  bool previous_state_{false};
  bool& flag_;
};

#ifdef COMET_MSVC
void TrackAlignedAlloc(void* ptr, std::size_t size);
void TrackAlignedFree(void* ptr);
#endif  // COMET_MSVC

void* MallocHooked(std::size_t size);
void* ReallocHooked(void* ptr, std::size_t size);
void* CallocHooked(std::size_t count, std::size_t size);
void FreeHooked(void* ptr);

#ifdef COMET_MSVC
extern void*(__cdecl* PlatformMalloc)(std::size_t);
extern void*(__cdecl* PlatformRealloc)(void*, std::size_t);
extern void*(__cdecl* PlatformCalloc)(std::size_t, std::size_t);
extern void(__cdecl* PlatformFree)(void*);

extern LPVOID(WINAPI* PlatformVirtualAlloc)(LPVOID, SIZE_T, DWORD, DWORD);
extern BOOL(WINAPI* PlatformVirtualFree)(LPVOID, SIZE_T, DWORD);

void* WINAPI VirtualAllocHooked(LPVOID lp_address, SIZE_T dw_size,
                                DWORD fl_allocation_type, DWORD fl_protect);
BOOL WINAPI VirtualFreeHooked(LPVOID lp_address, SIZE_T dw_size,
                              DWORD dw_free_type);
#else
extern void* (*PlatformMalloc)(std::size_t);
extern void* (*PlatformRealloc)(void*, std::size_t);
extern void* (*PlatformCalloc)(std::size_t, std::size_t);
extern void (*PlatformFree)(void*);

extern void* (*PlatformMmap)(void*, std::size_t, int, int, int, off_t);
extern int (*PlatformMunmap)(void*, std::size_t);
#endif  // COMET_MSVC

inline std::mutex latest_memory_snapshot_mutex{};
inline MemoryUseSnapshot latest_memory_snapshot{};
}  // namespace internal

void InitializeAllocationTracking();
void DestroyAllocationTracking();
void RegisterPlatformAllocation(void* ptr, usize size, MemoryTag memory_tag);
void RegisterPlatformDeallocation(void* ptr);
void RegisterTagAllocation(usize size, MemoryTag memory_tag);
void RegisterTagDeallocation(usize size, MemoryTag memory_tag);
void RegisterTaggedHeapPoolAllocation(usize size);
void RegisterTaggedHeapPoolDeallocation(usize size);
void RegisterTaggedHeapAllocation(usize size, MemoryTag memory_tag);
void RegisterTaggedHeapDeallocation(MemoryTag memory_tag);

usize GetTotalAllocatedMemory();
usize GetTotalFreedMemory();
usize GetMemoryUse();
Map<MemoryTag, usize> GetTagUse();
void UpdateMemoryUseSnapshot();
MemoryUseSnapshot GetLatestMemoryUseSnapshot();
}  // namespace memory
}  // namespace comet

#define COMET_INITIALIZE_ALLOCATION_TRACKING() \
  comet::memory::InitializeAllocationTracking()
#define COMET_DESTROY_ALLOCATION_TRACKING() \
  comet::memory::DestroyAllocationTracking()
#define COMET_REGISTER_PLATFORM_ALLOCATION(ptr, size, memory_tag) \
  comet::memory::RegisterPlatformAllocation(ptr, size, memory_tag)
#define COMET_REGISTER_PLATFORM_DEALLOCATION(ptr) \
  comet::memory::RegisterPlatformDeallocation(ptr)
#define COMET_REGISTER_TAG_ALLOCATION(size, memory_tag) \
  comet::memory::RegisterTagAllocation(size, memory_tag)
#define COMET_REGISTER_TAG_DEALLOCATION(size, memory_tag) \
  comet::memory::RegisterTagDeallocation(size, memory_tag)
#define COMET_REGISTER_TAGGED_HEAP_POOL_ALLOCATION(size) \
  comet::memory::RegisterTaggedHeapPoolAllocation(size)
#define COMET_REGISTER_TAGGED_HEAP_POOL_DEALLOCATION(size) \
  comet::memory::RegisterTaggedHeapPoolDeallocation(size)
#define COMET_REGISTER_TAGGED_HEAP_ALLOCATION(size, memory_tag) \
  comet::memory::RegisterTaggedHeapAllocation(size, memory_tag)
#define COMET_REGISTER_TAGGED_HEAP_DEALLOCATION(memory_tag) \
  comet::memory::RegisterTaggedHeapDeallocation(memory_tag)
#define COMET_GET_MEMORY_USE(handle) handle = comet::memory::GetMemoryUse()
#define COMET_GET_TAG_USE(handle) handle = comet::memory::GetTagUse()
#define COMET_UPDATE_MEMORY_USE_SNAPSHOT() \
  comet::memory::UpdateMemoryUseSnapshot()
#define COMET_GET_LATEST_MEMORY_USE_SNAPSHOT(handle) \
  handle = comet::memory::GetLatestMemoryUseSnapshot()

#ifdef COMET_MSVC
#endif  // COMET_MSVC
#else
#define COMET_INITIALIZE_ALLOCATION_TRACKING()
#define COMET_DESTROY_ALLOCATION_TRACKING()
#define COMET_REGISTER_PLATFORM_ALLOCATION(ptr, size, memory_tag)
#define COMET_REGISTER_PLATFORM_DEALLOCATION(ptr)
#define COMET_REGISTER_TAG_ALLOCATION(size, memory_tag)
#define COMET_REGISTER_TAG_DEALLOCATION(size, memory_tag)
#define COMET_REGISTER_TAGGED_HEAP_POOL_ALLOCATION(size)
#define COMET_REGISTER_TAGGED_HEAP_POOL_DEALLOCATION(size)
#define COMET_REGISTER_TAGGED_HEAP_ALLOCATION(size, memory_tag)
#define COMET_REGISTER_TAGGED_HEAP_DEALLOCATION(memory_tag)
#define COMET_GET_MEMORY_USE(handle)
#define COMET_GET_TAG_USE(handle)
#define COMET_UPDATE_MEMORY_USE_SNAPSHOT()
#define COMET_GET_LATEST_MEMORY_USE_SNAPSHOT(handle)
#endif  // COMET_TRACK_ALLOCATIONS

#endif  // COMET_RUNTIME_MEMORY_ALLOCATION_TRACKING_H_
