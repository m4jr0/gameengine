// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_ALLOCATION_H_
#define COMET_COMET_CORE_ALLOCATION_H_

#include "comet/core/define.h"

// TODO(m4jr0): Improve allocation tracking for MSVC and other compilers.
// Currently, tracking is partially functional on MSVC and not working properly
// on GCC. Tag allocation, however, appears to be functioning correctly in both
// cases.

#ifdef COMET_TRACK_ALLOCATIONS
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

#include "comet/core/essentials.h"
#include "comet/core/memory/memory.h"
#include "comet/math/math_commons.h"

#ifdef COMET_MSVC
#include "comet/core/windows.h"
#else
#include <dlfcn.h>
#include <sys/mman.h>
#endif  // COMET_MSVC

namespace comet {
namespace memory {
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
  void Push(void* ptr, usize size);
  usize Pop(void* ptr);

 private:
  std::recursive_mutex mutex_{};
  std::unordered_map<void*, usize> allocations_{};
};

// TODO(m4jr0): Consider using a lock-free solution (though it's primarily for
// debugging, so... yeah...).
struct TrackedTags {
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
  std::unordered_map<MemoryTag, usize> GetTagUse();

 private:
  std::shared_mutex platform_mutex_{};
  std::shared_mutex tagged_heap_mutex_{};
  std::unordered_map<void*, AllocationInfo> platform_allocations_{};
  std::unordered_map<MemoryTag, usize> platform_tags_{};
  std::unordered_map<MemoryTag, usize> tagged_heap_tags_{};
};

struct MemoryUse {
  static MemoryUse& Get();

  static_assert(std::atomic<usize>::is_always_lock_free,
                "std::atomic<usize> needs to be always lock-free. Unsupported "
                "architecture");

  std::atomic<usize> total_allocated{0};
  std::atomic<usize> total_freed{0};
  TrackedAllocations allocations{};
  TrackedTags tags{};
};

void* MallocHooked(std::size_t size);
void* ReallocHooked(void* ptr, std::size_t size);
void* CallocHooked(std::size_t count, std::size_t size);
void FreeHooked(void* ptr);

#ifdef COMET_MSVC
static void*(__cdecl* PlatformMalloc)(std::size_t) = std::malloc;
static void*(__cdecl* PlatformRealloc)(void*, std::size_t) = std::realloc;
static void*(__cdecl* PlatformCalloc)(std::size_t, std::size_t) = std::calloc;
static void(__cdecl* PlatformFree)(void*) = std::free;

static LPVOID(WINAPI* PlatformVirtualAlloc)(LPVOID, SIZE_T, DWORD,
                                            DWORD) = VirtualAlloc;
static BOOL(WINAPI* PlatformVirtualFree)(LPVOID, SIZE_T, DWORD) = VirtualFree;

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
std::unordered_map<MemoryTag, usize> GetTagUse();
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
#endif  // COMET_TRACK_ALLOCATIONS

#endif  // COMET_COMET_CORE_ALLOCATION_H_
