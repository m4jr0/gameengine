// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/memory/allocation_tracking.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_TRACK_ALLOCATIONS
// External. ///////////////////////////////////////////////////////////////////
#include <new>

#ifdef COMET_MSVC
#include "detours/detours.h"
#endif  // COMET_MSVC
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace memory {
namespace internal {
static thread_local bool is_tracking_in_progress{false};

TrackedAllocations::TrackedAllocations(memory::Allocator* allocator)
    : allocator{allocator} {}

TrackedAllocations::~TrackedAllocations() {
  COMET_ASSERT(!is_initialized, "TrackedAllocations::~TrackedAllocations",
               "tracked allocations are still initialized");
}

void TrackedAllocations::Initialize() {
  COMET_ASSERT(!is_initialized, "TrackedAllocations::Initialize",
               "tracked allocations are already initialized");
  COMET_ASSERT(allocator != nullptr, "TrackedAllocations::Initialize",
               "allocator is null");

  allocations = Map<void*, usize>{allocator};

  is_initialized = true;
}

void TrackedAllocations::Destroy() {
  COMET_ASSERT(is_initialized, "TrackedAllocations::Destroy",
               "tracked allocations are not initialized");

  allocations.Release();
  allocator = nullptr;

  is_initialized = false;
}

void TrackedAllocations::Push(void* ptr, usize size) {
  std::lock_guard lock{mutex};
  allocations.Set(ptr, size);
}

usize TrackedAllocations::Pop(void* ptr) {
  std::lock_guard lock{mutex};
  auto* size{allocations.TryGet(ptr)};

  if (size == nullptr) {
    return 0;
  }

  const auto to_return{*size};
  allocations.Remove(ptr);
  return to_return;
}

TrackedTags::TrackedTags(memory::Allocator* allocator) : allocator{allocator} {}

TrackedTags::~TrackedTags() {
  COMET_ASSERT(!is_initialized, "TrackedTags::~TrackedTags",
               "tracked tags are still initialized");
}

void TrackedTags::Initialize() {
  COMET_ASSERT(!is_initialized, "TrackedTags::Initialize",
               "tracked tags are already initialized");
  COMET_ASSERT(allocator != nullptr, "TrackedTags::Initialize",
               "allocator is null");

  platform_allocations = Map<void*, AllocationInfo>{allocator};
  platform_tags = Map<MemoryTag, usize>{allocator};
  tagged_heap_tags = Map<MemoryTag, usize>{allocator};

  is_initialized = true;
}

void TrackedTags::Destroy() {
  COMET_ASSERT(is_initialized, "TrackedTags::Destroy",
               "tracked tags are not initialized");

  platform_allocations.Release();
  platform_tags.Release();
  tagged_heap_tags.Release();
  allocator = nullptr;

  is_initialized = false;
}

void TrackedTags::IncreasePlatform(void* ptr, usize size,
                                   MemoryTag memory_tag) {
  std::unique_lock lock{platform_mutex};

  COMET_ASSERT(ptr != nullptr, "TrackedTags::IncreasePlatform",
               "tracked platform allocation pointer is null");
  COMET_ASSERT(!platform_allocations.IsContained(ptr),
               "TrackedTags::IncreasePlatform",
               "platform allocation is already tracked", "ptr", ptr, "size",
               size, "tag", memory_tag);

  platform_allocations.Set(ptr, AllocationInfo{size, memory_tag});
  platform_tags.GetOrAdd(memory_tag) += size;
}

void TrackedTags::DecreasePlatform(void* ptr) {
  std::unique_lock lock{platform_mutex};

  const auto alloc_info{platform_allocations.TryGet(ptr)};

  if (alloc_info == nullptr) {
    // COMET_ASSERT(false, "TrackedTags::DecreasePlatform",
    //              "platform allocation was not tracked", "ptr", ptr);
    return;
  }

  const auto size{alloc_info->size};
  const auto tag{alloc_info->tag};
  platform_allocations.Remove(ptr);
  platform_tags.GetOrAdd(tag) -= size;
}

void TrackedTags::IncreaseTag(usize size, MemoryTag memory_tag) {
  std::unique_lock lock{platform_mutex};
  platform_tags.GetOrAdd(memory_tag) += size;
}

void TrackedTags::DecreaseTag(usize size, MemoryTag memory_tag) {
  std::unique_lock lock{platform_mutex};
  platform_tags.GetOrAdd(memory_tag) -= size;
}

void TrackedTags::IncreaseTaggedHeapPool(usize size) {
  std::unique_lock lock{tagged_heap_mutex};
  tagged_heap_tags.GetOrAdd(kEngineMemoryTagTaggedHeap) += size;
}

void TrackedTags::DecreaseTaggedHeapPool(usize size) {
  std::unique_lock lock{tagged_heap_mutex};
  tagged_heap_tags.GetOrAdd(kEngineMemoryTagTaggedHeap) -= size;
}

void TrackedTags::IncreaseTaggedHeap(usize size, MemoryTag memory_tag) {
  std::unique_lock lock{tagged_heap_mutex};
  tagged_heap_tags.GetOrAdd(kEngineMemoryTagTaggedHeap) -= size;
  tagged_heap_tags.GetOrAdd(memory_tag) += size;
}

void TrackedTags::DecreaseTaggedHeap(MemoryTag memory_tag) {
  std::unique_lock lock{tagged_heap_mutex};
  tagged_heap_tags.GetOrAdd(kEngineMemoryTagTaggedHeap) +=
      tagged_heap_tags.GetOrAdd(memory_tag);
  tagged_heap_tags.Set(memory_tag, static_cast<usize>(0));
}

Map<MemoryTag, usize> TrackedTags::GetTagUse() {
  internal::ScopedFlagToggle toggle{is_tracking_in_progress};
  Map<MemoryTag, usize> info{allocator};

  {
    std::shared_lock lock{platform_mutex};

    for (const auto& pair : platform_tags) {
      info.Set(pair.key, pair.value);
    }
  }

  {
    std::shared_lock lock{tagged_heap_mutex};

    for (const auto& pair : tagged_heap_tags) {
      info.Set(pair.key, pair.value);
    }
  }

  return info;
}

#ifndef COMET_MSVC
std::once_flag malloc_init_flag{};
#endif  // !COMET_MSVC

void* MallocHooked(std::size_t size) {
#ifndef COMET_MSVC
  std::call_once(malloc_init_flag, []() {
    PlatformMalloc =
        reinterpret_cast<decltype(PlatformMalloc)>(dlsym(RTLD_NEXT, "malloc"));
    COMET_ASSERT(PlatformMalloc != nullptr, "memory::internal::MallocHooked",
                 "original malloc function could not be resolved");
  });
#endif  // !COMET_MSVC

  auto* ptr{PlatformMalloc(size)};

  if (ptr != nullptr && !is_tracking_in_progress) {
    internal::ScopedFlagToggle toggle{is_tracking_in_progress};
    MemoryUse::Get().allocations.Push(ptr, size);
    MemoryUse::Get().total_allocated.fetch_add(size, std::memory_order_relaxed);
  }

  return ptr;
}

#ifndef COMET_MSVC
std::once_flag realloc_init_flag{};
#endif  // !COMET_MSVC

void* ReallocHooked(void* ptr, std::size_t size) {
#ifndef COMET_MSVC
  std::call_once(realloc_init_flag, []() {
    PlatformRealloc = reinterpret_cast<decltype(PlatformRealloc)>(
        dlsym(RTLD_NEXT, "realloc"));
    COMET_ASSERT(PlatformRealloc != nullptr, "memory::internal::ReallocHooked",
                 "original realloc function could not be resolved");
  });
#endif  // !COMET_MSVC

  auto* new_ptr{PlatformRealloc(ptr, size)};

  if (!is_tracking_in_progress) {
    internal::ScopedFlagToggle toggle{is_tracking_in_progress};

    if (ptr != nullptr && (new_ptr != nullptr || size == 0)) {
      MemoryUse::Get().total_freed.fetch_add(
          MemoryUse::Get().allocations.Pop(ptr), std::memory_order_relaxed);
    }

    if (new_ptr != nullptr) {
      MemoryUse::Get().allocations.Push(new_ptr, size);
      MemoryUse::Get().total_allocated.fetch_add(size,
                                                 std::memory_order_relaxed);
    }
  }

  return new_ptr;
}

#ifndef COMET_MSVC
std::once_flag calloc_init_flag{};
#endif  // !COMET_MSVC

void* CallocHooked(std::size_t count, std::size_t size) {
#ifndef COMET_MSVC
  std::call_once(calloc_init_flag, []() {
    PlatformCalloc =
        reinterpret_cast<decltype(PlatformCalloc)>(dlsym(RTLD_NEXT, "calloc"));
    COMET_ASSERT(PlatformCalloc != nullptr, "memory::internal::CallocHooked",
                 "original calloc function could not be resolved");
  });
#endif  // !COMET_MSVC

  auto* ptr{PlatformCalloc(count, size)};

  if (ptr != nullptr && !is_tracking_in_progress) {
    internal::ScopedFlagToggle toggle{is_tracking_in_progress};

    MemoryUse::Get().allocations.Push(ptr, count * size);
    MemoryUse::Get().total_allocated.fetch_add(count * size,
                                               std::memory_order_relaxed);
  }

  return ptr;
}

#ifndef COMET_MSVC
std::once_flag free_init_flag{};
#endif  // !COMET_MSVC

void FreeHooked(void* ptr) {
#ifndef COMET_MSVC
  std::call_once(free_init_flag, []() {
    PlatformFree =
        reinterpret_cast<decltype(PlatformFree)>(dlsym(RTLD_NEXT, "free"));
    COMET_ASSERT(PlatformFree != nullptr, "memory::internal::FreeHooked",
                 "original free function could not be resolved");
  });
#endif  // !COMET_MSVC

  auto* platform_free{PlatformFree};

  if (platform_free == nullptr) {
    std::free(ptr);
    return;
  }

  if (is_tracking_in_progress) {
    platform_free(ptr);
    return;
  }

  auto& memory_use{MemoryUse::Get()};

  if (!memory_use.is_tracking) {
    platform_free(ptr);
    return;
  }

  internal::ScopedFlagToggle toggle{is_tracking_in_progress};

  if (ptr != nullptr) {
    memory_use.total_freed.fetch_add(memory_use.allocations.Pop(ptr),
                                     std::memory_order_relaxed);
  }

  platform_free(ptr);
}

#ifdef COMET_MSVC
void*(__cdecl* PlatformMalloc)(std::size_t) = std::malloc;
void*(__cdecl* PlatformRealloc)(void*, std::size_t) = std::realloc;
void*(__cdecl* PlatformCalloc)(std::size_t, std::size_t) = std::calloc;
void(__cdecl* PlatformFree)(void*) = std::free;

LPVOID(WINAPI* PlatformVirtualAlloc)(LPVOID, SIZE_T, DWORD,
                                     DWORD) = VirtualAlloc;
BOOL(WINAPI* PlatformVirtualFree)(LPVOID, SIZE_T, DWORD) = VirtualFree;

void* WINAPI VirtualAllocHooked(LPVOID lp_address, SIZE_T dw_size,
                                DWORD fl_allocation_type, DWORD fl_protect) {
  auto* result{PlatformVirtualAlloc(lp_address, dw_size, fl_allocation_type,
                                    fl_protect)};

  if (result != nullptr && (fl_allocation_type & MEM_COMMIT) != 0 &&
      !is_tracking_in_progress) {
    internal::ScopedFlagToggle toggle{is_tracking_in_progress};
    MemoryUse::Get().allocations.Push(result, dw_size);
    MemoryUse::Get().total_allocated.fetch_add(dw_size,
                                               std::memory_order_relaxed);
  }

  return result;
}

BOOL WINAPI VirtualFreeHooked(LPVOID lp_address, SIZE_T dw_size,
                              DWORD dw_free_type) {
  const auto result{PlatformVirtualFree(lp_address, dw_size, dw_free_type)};

  if (result && lp_address != nullptr && !is_tracking_in_progress &&
      ((dw_free_type & MEM_RELEASE) != 0 ||
       (dw_free_type & MEM_DECOMMIT) != 0)) {
    internal::ScopedFlagToggle toggle{is_tracking_in_progress};

    MemoryUse::Get().total_freed.fetch_add(
        MemoryUse::Get().allocations.Pop(lp_address),
        std::memory_order_relaxed);
  }

  return result;
}
#else
std::once_flag mmap_init_flag{};

void* MmapHooked(void* addr, size_t len, int prot, int flags, int fd,
                 off_t offset) {
  std::call_once(mmap_init_flag, []() {
    PlatformMmap =
        reinterpret_cast<decltype(PlatformMmap)>(dlsym(RTLD_NEXT, "mmap"));
    COMET_ASSERT(PlatformMmap != nullptr, "memory::internal::MmapHooked",
                 "original mmap function could not be resolved");
  });

  auto* ptr{PlatformMmap(addr, len, prot, flags, fd, offset)};

  if (is_tracking_in_progress) {
    return ptr;
  }

  internal::ScopedFlagToggle toggle{is_tracking_in_progress};

  if (ptr != MAP_FAILED) {
    MemoryUse::Get().allocations.Push(ptr, len);
    MemoryUse::Get().total_allocated.fetch_add(len, std::memory_order_relaxed);
  }

  return ptr;
}

std::once_flag munmap_init_flag{};

int munmapHooked(void* addr, size_t len) {
  std::call_once(munmap_init_flag, []() {
    PlatformMunmap =
        reinterpret_cast<decltype(PlatformMunmap)>(dlsym(RTLD_NEXT, "munmap"));
    COMET_ASSERT(PlatformMunmap != nullptr, "memory::internal::munmapHooked",
                 "original munmap function could not be resolved");
  });

  const auto result{PlatformMunmap(addr, len)};

  if (is_tracking_in_progress) {
    return result;
  }

  internal::ScopedFlagToggle toggle{is_tracking_in_progress};

  if (result == 0) {
    MemoryUse::Get().total_freed.fetch_add(
        MemoryUse::Get().allocations.Pop(addr), std::memory_order_relaxed);
  }

  return result;
}
#endif  // COMET_MSVC

MemoryUse& MemoryUse::Get() {
  static MemoryUse* singleton{nullptr};

  if (singleton == nullptr) {
    singleton = new MemoryUse();
  }

  return *singleton;
}

MemoryUse::~MemoryUse() {
  COMET_ASSERT(!is_initialized, "MemoryUse::~MemoryUse",
               "memory use tracking is still initialized");
}

void MemoryUse::Initialize() {
  COMET_ASSERT(!is_initialized, "MemoryUse::Initialize",
               "memory use tracking is already initialized");

  allocations.Initialize();
  tags.Initialize();

  is_initialized = true;
}

void MemoryUse::Destroy() {
  COMET_ASSERT(is_initialized, "MemoryUse::Destroy",
               "memory use tracking is not initialized");

  allocations.Destroy();
  tags.Destroy();

  is_initialized = false;
}

ScopedFlagToggle::ScopedFlagToggle(bool& flag)
    : previous_state_{flag}, flag_{flag} {
  flag_ = true;
}

ScopedFlagToggle::~ScopedFlagToggle() { flag_ = previous_state_; }

#ifndef COMET_MSVC
void* (*PlatformMalloc)(std::size_t){nullptr};
void* (*PlatformRealloc)(void*, std::size_t){nullptr};
void* (*PlatformCalloc)(std::size_t, std::size_t){nullptr};
void (*PlatformFree)(void*){nullptr};

void* (*PlatformMmap)(void*, std::size_t, int, int, int, off_t){nullptr};
int (*PlatformMunmap)(void*, std::size_t){nullptr};
#endif  // !COMET_MSVC

#ifdef COMET_MSVC
void TrackAlignedAlloc(void* ptr, std::size_t size) {
  if (ptr == nullptr || comet::memory::internal::is_tracking_in_progress) {
    return;
  }

  ScopedFlagToggle toggle{is_tracking_in_progress};

  MemoryUse::Get().allocations.Push(ptr, size);
  MemoryUse::Get().total_allocated.fetch_add(size, std::memory_order_relaxed);
}

void TrackAlignedFree(void* ptr) {
  if (ptr == nullptr || comet::memory::internal::is_tracking_in_progress) {
    return;
  }

  ScopedFlagToggle toggle{is_tracking_in_progress};

  MemoryUse::Get().total_freed.fetch_add(MemoryUse::Get().allocations.Pop(ptr),
                                         std::memory_order_relaxed);
}
#endif  // COMET_MSVC
}  // namespace internal

void InitializeAllocationTracking() {
  auto& memory_use{internal::MemoryUse::Get()};

  if (memory_use.is_tracking) {
    return;
  }

  memory_use.Initialize();

#ifdef COMET_MSVC
  DetourTransactionBegin();
  DetourUpdateThread(GetCurrentThread());

  DetourAttach(&(PVOID&)internal::PlatformMalloc, internal::MallocHooked);
  DetourAttach(&(PVOID&)internal::PlatformRealloc, internal::ReallocHooked);
  DetourAttach(&(PVOID&)internal::PlatformCalloc, internal::CallocHooked);
  DetourAttach(&(PVOID&)internal::PlatformFree, internal::FreeHooked);

  DetourAttach(&reinterpret_cast<PVOID&>(internal::PlatformVirtualAlloc),
               internal::VirtualAllocHooked);
  DetourAttach(&reinterpret_cast<PVOID&>(internal::PlatformVirtualFree),
               internal::VirtualFreeHooked);

  DetourTransactionCommit();
#else
  // TODO(m4jr0): Non-MSVC platforms.
#endif  // COMET_MSVC

  memory_use.is_tracking = true;
}

void DestroyAllocationTracking() {
  auto& memory_use{internal::MemoryUse::Get()};

  if (!memory_use.is_tracking) {
    return;
  }

  memory_use.is_tracking = false;

#ifdef COMET_MSVC
  DetourTransactionBegin();
  DetourUpdateThread(GetCurrentThread());

  DetourDetach(&(PVOID&)internal::PlatformMalloc, internal::MallocHooked);
  DetourDetach(&(PVOID&)internal::PlatformRealloc, internal::ReallocHooked);
  DetourDetach(&(PVOID&)internal::PlatformCalloc, internal::CallocHooked);
  DetourDetach(&(PVOID&)internal::PlatformFree, internal::FreeHooked);

  DetourDetach(&reinterpret_cast<PVOID&>(internal::PlatformVirtualAlloc),
               internal::VirtualAllocHooked);
  DetourDetach(&reinterpret_cast<PVOID&>(internal::PlatformVirtualFree),
               internal::VirtualFreeHooked);

  DetourTransactionCommit();
#else
  // TODO(m4jr0): Non-MSVC platforms.
#endif  // COMET_MSVC

  memory_use.Destroy();
}

void RegisterPlatformAllocation(void* ptr, usize size, MemoryTag memory_tag) {
  auto& memory_use{internal::MemoryUse::Get()};

  if (!memory_use.is_tracking || internal::is_tracking_in_progress) {
    return;
  }

  internal::ScopedFlagToggle flag{internal::is_tracking_in_progress};
  memory_use.tags.IncreasePlatform(ptr, size, memory_tag);
}

void RegisterPlatformDeallocation(void* ptr) {
  auto& memory_use{internal::MemoryUse::Get()};

  if (!memory_use.is_tracking || internal::is_tracking_in_progress) {
    return;
  }

  internal::ScopedFlagToggle flag{internal::is_tracking_in_progress};
  memory_use.tags.DecreasePlatform(ptr);
}

void RegisterTagAllocation(usize size, MemoryTag memory_tag) {
  auto& memory_use{internal::MemoryUse::Get()};

  if (!memory_use.is_tracking || internal::is_tracking_in_progress) {
    return;
  }

  internal::ScopedFlagToggle flag{internal::is_tracking_in_progress};
  memory_use.tags.IncreaseTag(size, memory_tag);
}

void RegisterTagDeallocation(usize size, MemoryTag memory_tag) {
  auto& memory_use{internal::MemoryUse::Get()};

  if (!memory_use.is_tracking || internal::is_tracking_in_progress) {
    return;
  }

  internal::ScopedFlagToggle flag{internal::is_tracking_in_progress};
  memory_use.tags.DecreaseTag(size, memory_tag);
}

void RegisterTaggedHeapPoolAllocation(usize size) {
  auto& memory_use{internal::MemoryUse::Get()};

  if (!memory_use.is_tracking || internal::is_tracking_in_progress) {
    return;
  }

  internal::ScopedFlagToggle flag{internal::is_tracking_in_progress};
  memory_use.tags.IncreaseTaggedHeapPool(size);
}

void RegisterTaggedHeapPoolDeallocation(usize size) {
  auto& memory_use{internal::MemoryUse::Get()};

  if (!memory_use.is_tracking || internal::is_tracking_in_progress) {
    return;
  }

  internal::ScopedFlagToggle flag{internal::is_tracking_in_progress};
  memory_use.tags.DecreaseTaggedHeapPool(size);
}

void RegisterTaggedHeapAllocation(usize size, MemoryTag memory_tag) {
  auto& memory_use{internal::MemoryUse::Get()};

  if (!memory_use.is_tracking || internal::is_tracking_in_progress) {
    return;
  }

  internal::ScopedFlagToggle flag{internal::is_tracking_in_progress};
  memory_use.tags.IncreaseTaggedHeap(size, memory_tag);
}

void RegisterTaggedHeapDeallocation(MemoryTag memory_tag) {
  auto& memory_use{internal::MemoryUse::Get()};

  if (!memory_use.is_tracking || internal::is_tracking_in_progress) {
    return;
  }

  internal::ScopedFlagToggle flag{internal::is_tracking_in_progress};
  memory_use.tags.DecreaseTaggedHeap(memory_tag);
}

usize GetTotalAllocatedMemory() {
  return internal::MemoryUse::Get().total_allocated;
}

usize GetTotalFreedMemory() { return internal::MemoryUse::Get().total_freed; }

usize GetMemoryUse() {
  COMET_ASSERT(internal::MemoryUse::Get().total_allocated >=
                   internal::MemoryUse::Get().total_freed,
               "memory::GetMemoryUse", "freed memory exceeds allocated memory",
               "total_allocated",
               internal::MemoryUse::Get().total_allocated.load(), "total_freed",
               internal::MemoryUse::Get().total_freed.load());
  return internal::MemoryUse::Get().total_allocated -
         internal::MemoryUse::Get().total_freed;
}

// TODO(m4jr0): Consider using a thread-unsafe function.
// It might be suitable at specific points to avoid locking overhead.
Map<MemoryTag, usize> GetTagUse() {
  return internal::MemoryUse::Get().tags.GetTagUse();
}

void UpdateMemoryUseSnapshot() {
  auto tag_use{GetTagUse()};

  MemoryUseSnapshot snapshot{};
  snapshot.memory_use = GetMemoryUse();

  for (const auto& pair : tag_use) {
    if (snapshot.tag_count >= MemoryUseSnapshot::kMaxTagCount) {
      break;
    }

    snapshot.tags[snapshot.tag_count++] = {pair.key, pair.value};
  }

  std::lock_guard lock{internal::latest_memory_snapshot_mutex};
  internal::latest_memory_snapshot = snapshot;
}

MemoryUseSnapshot GetLatestMemoryUseSnapshot() {
  std::lock_guard lock{internal::latest_memory_snapshot_mutex};
  return internal::latest_memory_snapshot;
}
}  // namespace memory
}  // namespace comet

#ifdef COMET_MSVC
_VCRT_EXPORT_STD _NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size)
_VCRT_ALLOCATOR void* __CRTDECL operator new(std::size_t size) {
  if (size == 0) {
    size = 1;
  }

  if (void* ptr{std::malloc(size)}) {
    return ptr;
  }

  throw std::bad_alloc();
}

_VCRT_EXPORT_STD _NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size)
_VCRT_ALLOCATOR void* __CRTDECL operator new[](std::size_t size) {
  if (size == 0) {
    size = 1;
  }

  if (auto* ptr{std::malloc(size)}) {
    return ptr;
  }

  throw std::bad_alloc();
}
#else
void* operator new(std::size_t size) {
  if (size == 0) {
    size = 1;
  }

  if (void* ptr{std::malloc(size)}) {
    return ptr;
  }

  throw std::bad_alloc();
}

void* operator new[](std::size_t size) {
  if (size == 0) {
    size = 1;
  }

  if (auto* ptr{std::malloc(size)}) {
    return ptr;
  }

  throw std::bad_alloc();
}
#endif  // COMET_MSVC

#ifdef __cpp_aligned_new
#ifdef COMET_MSVC
_VCRT_EXPORT_STD _NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size)
_VCRT_ALLOCATOR
void* __CRTDECL operator new(std::size_t size, std::align_val_t align) {
  if (size == 0) {
    size = 1;
  }

  auto* ptr{_aligned_malloc(size, static_cast<std::size_t>(align))};

  if (ptr != nullptr) {
    comet::memory::internal::TrackAlignedAlloc(ptr, size);
    return ptr;
  }

  throw std::bad_alloc();
}

_VCRT_EXPORT_STD _NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size)
_VCRT_ALLOCATOR
void* __CRTDECL operator new[](std::size_t size, std::align_val_t align) {
  if (size == 0) {
    size = 1;
  }

  auto* ptr{_aligned_malloc(size, static_cast<std::size_t>(align))};

  if (ptr != nullptr) {
    comet::memory::internal::TrackAlignedAlloc(ptr, size);
    return ptr;
  }

  throw std::bad_alloc();
}

_VCRT_EXPORT_STD _NODISCARD _Ret_maybenull_ _Success_(return != NULL)
    _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
    void* __CRTDECL operator new(std::size_t size,
                                 std::nothrow_t const&) noexcept {
  if (size == 0) {
    size = 1;
  }

  return std::malloc(size);
}

_VCRT_EXPORT_STD _NODISCARD _Ret_maybenull_ _Success_(return != NULL)
    _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
    void* __CRTDECL operator new[](std::size_t size,
                                   ::std::nothrow_t const&) noexcept {
  if (size == 0) {
    size = 1;
  }

  return std::malloc(size);
}

_VCRT_EXPORT_STD _NODISCARD _Ret_maybenull_ _Success_(return != NULL)
    _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
    void* __CRTDECL operator new(std::size_t size, std::align_val_t align,
                                 const std::nothrow_t&) noexcept {
  if (size == 0) {
    size = 1;
  }

  auto* ptr{_aligned_malloc(size, static_cast<std::size_t>(align))};
  comet::memory::internal::TrackAlignedAlloc(ptr, size);
  return ptr;
}

_VCRT_EXPORT_STD _NODISCARD _Ret_maybenull_ _Success_(return != NULL)
    _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
    void* __CRTDECL operator new[](std::size_t size, std::align_val_t align,
                                   const std::nothrow_t&) noexcept {
  if (size == 0) {
    size = 1;
  }

  auto* ptr{_aligned_malloc(size, static_cast<std::size_t>(align))};
  comet::memory::internal::TrackAlignedAlloc(ptr, size);
  return ptr;
}
#else
void* operator new(std::size_t size, std::align_val_t align) {
  if (size == 0) {
    size = 1;
  }

  const auto alignment{static_cast<std::size_t>(align)};
  const auto aligned_size{((size + alignment - 1) / alignment) * alignment};

  if (void* ptr{std::aligned_alloc(alignment, aligned_size)}) {
    return ptr;
  }

  throw std::bad_alloc();
}

void* operator new[](std::size_t size, std::align_val_t align) {
  if (size == 0) {
    size = 1;
  }

  const auto alignment{static_cast<std::size_t>(align)};
  const auto aligned_size{((size + alignment - 1) / alignment) * alignment};

  if (void* ptr{std::aligned_alloc(alignment, aligned_size)}) {
    return ptr;
  }

  throw std::bad_alloc();
}

void* operator new(std::size_t size, const std::nothrow_t&) noexcept {
  if (size == 0) {
    size = 1;
  }

  return std::malloc(size);
}

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept {
  if (size == 0) {
    size = 1;
  }

  return std::malloc(size);
}

void* operator new(std::size_t size, std::align_val_t align,
                   const std::nothrow_t&) noexcept {
  if (size == 0) {
    size = 1;
  }

  const auto alignment{static_cast<std::size_t>(align)};
  const auto aligned_size{((size + alignment - 1) / alignment) * alignment};

  return std::aligned_alloc(alignment, aligned_size);
}

void* operator new[](std::size_t size, std::align_val_t align,
                     const std::nothrow_t&) noexcept {
  if (size == 0) {
    size = 1;
  }

  const auto alignment{static_cast<std::size_t>(align)};
  const auto aligned_size{((size + alignment - 1) / alignment) * alignment};

  return std::aligned_alloc(alignment, aligned_size);
}
#endif  // COMET_MSVC
#endif  // __cpp_aligned_new

void operator delete(void* ptr, std::size_t) noexcept { std::free(ptr); }

void operator delete[](void* ptr, std::size_t) noexcept { std::free(ptr); }

void operator delete(void* ptr) noexcept { std::free(ptr); }

void operator delete[](void* ptr) noexcept { std::free(ptr); }

void operator delete(void* ptr, const std::nothrow_t&) noexcept {
  std::free(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t&) noexcept {
  std::free(ptr);
}

#ifdef __cpp_aligned_new
#ifdef COMET_MSVC
void operator delete(void* ptr, std::align_val_t) noexcept {
  comet::memory::internal::TrackAlignedFree(ptr);
  _aligned_free(ptr);
}

void operator delete[](void* ptr, std::align_val_t) noexcept {
  comet::memory::internal::TrackAlignedFree(ptr);
  _aligned_free(ptr);
}

void operator delete(void* ptr, std::size_t, std::align_val_t) noexcept {
  comet::memory::internal::TrackAlignedFree(ptr);
  _aligned_free(ptr);
}

void operator delete[](void* ptr, std::size_t, std::align_val_t) noexcept {
  comet::memory::internal::TrackAlignedFree(ptr);
  _aligned_free(ptr);
}

void operator delete(void* ptr, std::align_val_t,
                     const std::nothrow_t&) noexcept {
  comet::memory::internal::TrackAlignedFree(ptr);
  _aligned_free(ptr);
}

void operator delete[](void* ptr, std::align_val_t,
                       const std::nothrow_t&) noexcept {
  comet::memory::internal::TrackAlignedFree(ptr);
  _aligned_free(ptr);
}
#else
void operator delete(void* ptr, std::align_val_t) noexcept { std::free(ptr); }

void operator delete[](void* ptr, std::align_val_t) noexcept { std::free(ptr); }

void operator delete(void* ptr, std::size_t, std::align_val_t) noexcept {
  std::free(ptr);
}

void operator delete[](void* ptr, std::size_t, std::align_val_t) noexcept {
  std::free(ptr);
}

void operator delete(void* ptr, std::align_val_t,
                     const std::nothrow_t&) noexcept {
  std::free(ptr);
}

void operator delete[](void* ptr, std::align_val_t,
                       const std::nothrow_t&) noexcept {
  std::free(ptr);
}
#endif  // COMET_MSVC
#endif  // __cpp_aligned_new

#endif  // COMET_TRACK_ALLOCATIONS