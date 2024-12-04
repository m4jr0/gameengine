// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "allocation_tracking.h"

#ifdef COMET_TRACK_ALLOCATIONS
#include <new>

#ifdef COMET_MSVC
#include "detours/detours.h"
#endif  // COMET_MSVC

namespace comet {
namespace memory {
namespace internal {
void TrackedAllocations::Push(void* ptr, usize size) {
  std::lock_guard lock{mutex_};
  allocations_[ptr] = size;
}

usize TrackedAllocations::Pop(void* ptr) {
  std::lock_guard lock{mutex_};
  auto it{allocations_.find(ptr)};

  if (it == allocations_.end()) {
    return 0;
  }

  auto size{it->second};
  allocations_.erase(it);
  return size;
}

void TrackedTags::IncreasePlatform(void* ptr, usize size,
                                   MemoryTag memory_tag) {
  std::unique_lock lock{platform_mutex_};
  platform_allocations_[ptr] = {size, memory_tag};
  platform_tags_[memory_tag] += size;
}

void TrackedTags::DecreasePlatform(void* ptr) {
  std::unique_lock lock{platform_mutex_};
  auto it = platform_allocations_.find(ptr);

  if (it == platform_allocations_.end()) {
    return;
  }

  auto size{it->second.size};
  auto tag{it->second.tag};

  platform_allocations_.erase(it);
  platform_tags_[tag] -= size;
}

void TrackedTags::IncreaseTag(usize size, MemoryTag memory_tag) {
  std::unique_lock lock{platform_mutex_};
  platform_tags_[memory_tag] += size;
}

void TrackedTags::DecreaseTag(usize size, MemoryTag memory_tag) {
  std::unique_lock lock{platform_mutex_};
  platform_tags_[memory_tag] -= size;
}

void TrackedTags::IncreaseTaggedHeapPool(usize size) {
  std::unique_lock lock{tagged_heap_mutex_};
  tagged_heap_tags_[kEngineMemoryTagTaggedHeap] += size;
}

void TrackedTags::DecreaseTaggedHeapPool(usize size) {
  std::unique_lock lock{tagged_heap_mutex_};
  tagged_heap_tags_[kEngineMemoryTagTaggedHeap] -= size;
}

void TrackedTags::IncreaseTaggedHeap(usize size, MemoryTag memory_tag) {
  std::unique_lock lock{tagged_heap_mutex_};
  tagged_heap_tags_[kEngineMemoryTagTaggedHeap] -= size;
  tagged_heap_tags_[memory_tag] += size;
}

void TrackedTags::DecreaseTaggedHeap(MemoryTag memory_tag) {
  std::unique_lock lock{tagged_heap_mutex_};
  tagged_heap_tags_[kEngineMemoryTagTaggedHeap] +=
      tagged_heap_tags_[memory_tag];
  tagged_heap_tags_[memory_tag] = 0;
}

std::unordered_map<MemoryTag, usize> TrackedTags::GetTagUse() {
  std::unordered_map<MemoryTag, usize> info{};

  {
    std::shared_lock lock{platform_mutex_};

    for (const auto& pair : platform_tags_) {
      info[pair.first] = pair.second;
    }
  }

  {
    std::shared_lock lock{tagged_heap_mutex_};

    for (const auto& pair : tagged_heap_tags_) {
      info[pair.first] = pair.second;
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
    COMET_ASSERT(PlatformMalloc != nullptr,
                 "Could not locate the original malloc function!");
  });
#endif  // !COMET_MSVC

  auto* ptr{PlatformMalloc(size)};

  if (ptr != nullptr) {
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
    COMET_ASSERT(PlatformRealloc != nullptr,
                 "Could not locate the original realloc function!");
  });
#endif  // !COMET_MSVC

  auto* new_ptr{PlatformRealloc(ptr, size)};

  if (new_ptr != nullptr) {
    if (ptr != nullptr) {
      MemoryUse::Get().total_freed.fetch_add(
          MemoryUse::Get().allocations.Pop(ptr), std::memory_order_relaxed);
    }

    MemoryUse::Get().total_allocated.fetch_add(size, std::memory_order_relaxed);
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
    COMET_ASSERT(PlatformCalloc != nullptr,
                 "Could not locate the original calloc function!");
  });
#endif  // !COMET_MSVC

  auto* ptr{PlatformCalloc(count, size)};

  if (ptr != nullptr) {
    MemoryUse::Get().total_allocated.fetch_add(count * size,
                                               std::memory_order_relaxed);
  }

  return ptr;
}

#ifndef COMET_MSVC
std::once_flag free_init_flag{};
#endif  // !COMET_MSVC

void FreeHooked(void* ptr) {
  if (ptr != nullptr) {
    MemoryUse::Get().total_freed.fetch_add(
        MemoryUse::Get().allocations.Pop(ptr), std::memory_order_relaxed);
  }

#ifndef COMET_MSVC
  std::call_once(free_init_flag, []() {
    PlatformFree =
        reinterpret_cast<decltype(PlatformFree)>(dlsym(RTLD_NEXT, "free"));
    COMET_ASSERT(PlatformFree != nullptr,
                 "Could not locate the original free function!");
  });
#endif  // !COMET_MSVC
  PlatformFree(ptr);
}

#ifdef COMET_MSVC
void* WINAPI VirtualAllocHooked(LPVOID lp_address, SIZE_T dw_size,
                                DWORD fl_allocation_type, DWORD fl_protect) {
  auto* result{PlatformVirtualAlloc(lp_address, dw_size, fl_allocation_type,
                                    fl_protect)};

  if ((fl_allocation_type & MEM_COMMIT) != 0) {
    MemoryUse::Get().allocations.Push(result, dw_size);
    MemoryUse::Get().total_allocated.fetch_add(dw_size,
                                               std::memory_order_relaxed);
  }

  return result;
}

BOOL WINAPI VirtualFreeHooked(LPVOID lp_address, SIZE_T dw_size,
                              DWORD dw_free_type) {
  auto result{PlatformVirtualFree(lp_address, dw_size, dw_free_type)};

  if (result) {
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
    COMET_ASSERT(PlatformMmap != nullptr,
                 "Could not locate the original mmap function!");
  });

  auto* ptr{PlatformMmap(addr, len, prot, flags, fd, offset)};

  if (ptr != MAP_FAILED) {
    MemoryUse::Get().allocations.Push(addr, len);
    MemoryUse::Get().total_allocated.fetch_add(len, std::memory_order_relaxed);
  }

  return ptr;
}

std::once_flag munmap_init_flag{};

int munmapHooked(void* addr, size_t len) {
  std::call_once(munmap_init_flag, []() {
    PlatformMunmap =
        reinterpret_cast<decltype(PlatformMunmap)>(dlsym(RTLD_NEXT, "munmap"));
    COMET_ASSERT(PlatformMunmap != nullptr,
                 "Could not locate the original munmap function!");
  });

  auto result{PlatformMunmap(addr, len)};

  if (result == 0) {
    MemoryUse::Get().total_freed.fetch_add(
        MemoryUse::Get().allocations.Pop(addr), std::memory_order_relaxed);
  }

  return result;
}
#endif
// COMET_MSVC

MemoryUse& MemoryUse::Get() {
  static MemoryUse* singleton{nullptr};

  if (singleton == nullptr) {
    singleton = new MemoryUse();
  }

  return *singleton;
}

#ifndef COMET_MSVC
void* (*PlatformMalloc)(std::size_t){nullptr};
void* (*PlatformRealloc)(void*, std::size_t){nullptr};
void* (*PlatformCalloc)(std::size_t, std::size_t){nullptr};
void (*PlatformFree)(void*){nullptr};

void* (*PlatformMmap)(void*, std::size_t, int, int, int, off_t){nullptr};
int (*PlatformMunmap)(void*, std::size_t){nullptr};
#endif  // !COMET_MSVC
}  // namespace internal

void InitializeAllocationTracking() {
  internal::MemoryUse::Get();

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
}

void DestroyAllocationTracking() {
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
}

void RegisterPlatformAllocation(void* ptr, usize size, MemoryTag memory_tag) {
  internal::MemoryUse::Get().tags.IncreasePlatform(ptr, size, memory_tag);
}

void RegisterPlatformDeallocation(void* ptr) {
  internal::MemoryUse::Get().tags.DecreasePlatform(ptr);
}

void RegisterTagAllocation(usize size, MemoryTag memory_tag) {
  internal::MemoryUse::Get().tags.IncreaseTag(size, memory_tag);
}

void RegisterTagDeallocation(usize size, MemoryTag memory_tag) {
  internal::MemoryUse::Get().tags.DecreaseTag(size, memory_tag);
}

void RegisterTaggedHeapPoolAllocation(usize size) {
  internal::MemoryUse::Get().tags.IncreaseTaggedHeapPool(size);
}

void RegisterTaggedHeapPoolDeallocation(usize size) {
  internal::MemoryUse::Get().tags.DecreaseTaggedHeapPool(size);
}

void RegisterTaggedHeapAllocation(usize size, MemoryTag memory_tag) {
  internal::MemoryUse::Get().tags.IncreaseTaggedHeap(size, memory_tag);
}

void RegisterTaggedHeapDeallocation(MemoryTag memory_tag) {
  internal::MemoryUse::Get().tags.DecreaseTaggedHeap(memory_tag);
}

usize GetTotalAllocatedMemory() {
  return internal::MemoryUse::Get().total_allocated;
}

usize GetTotalFreedMemory() { return internal::MemoryUse::Get().total_freed; }

usize GetMemoryUse() {
  COMET_ASSERT(internal::MemoryUse::Get().total_allocated >=
                   internal::MemoryUse::Get().total_freed,
               "Something wrong happened while allocating memory!");
  return internal::MemoryUse::Get().total_allocated -
         internal::MemoryUse::Get().total_freed;
}

// TODO(m4jr0): Consider using a thread-unsafe function.
// It might be suitable at specific points to avoid locking overhead.
std::unordered_map<MemoryTag, usize> GetTagUse() {
  return internal::MemoryUse::Get().tags.GetTagUse();
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
_VCRT_ALLOCATOR void* __CRTDECL operator new(std::size_t size,
                                             std::align_val_t align) {
  if (size == 0) {
    size = 1;
  }

  if (void* ptr{_aligned_malloc(size, static_cast<std::size_t>(align))}) {
    return ptr;
  }

  throw std::bad_alloc();
}

_VCRT_EXPORT_STD _NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size)
_VCRT_ALLOCATOR void* __CRTDECL operator new[](std::size_t size,
                                               std::align_val_t align) {
  if (size == 0) {
    size = 1;
  }

  if (void* ptr{_aligned_malloc(size, static_cast<std::size_t>(align))}) {
    return ptr;
  }

  throw std::bad_alloc();
}

_VCRT_EXPORT_STD _NODISCARD _Ret_maybenull_ _Success_(return != NULL)
    _Post_writable_byte_size_(size) _VCRT_ALLOCATOR void* __CRTDECL
    operator new(std::size_t size, std::nothrow_t const&) noexcept {
  if (size == 0) {
    size = 1;
  }

  return std::malloc(size);
}

_VCRT_EXPORT_STD _NODISCARD _Ret_maybenull_ _Success_(return != NULL)
    _Post_writable_byte_size_(size) _VCRT_ALLOCATOR void* __CRTDECL
    operator new[](size_t size, ::std::nothrow_t const&) noexcept {
  if (size == 0) {
    size = 1;
  }

  return std::malloc(size);
}

_VCRT_EXPORT_STD _NODISCARD _Ret_maybenull_ _Success_(return != NULL)
    _Post_writable_byte_size_(size) _VCRT_ALLOCATOR void* __CRTDECL
    operator new(std::size_t size, std::align_val_t align,
                 const std::nothrow_t&) noexcept {
  if (size == 0) {
    size = 1;
  }

  return _aligned_malloc(size, static_cast<std::size_t>(align));
}

_VCRT_EXPORT_STD _NODISCARD _Ret_maybenull_ _Success_(return != NULL)
    _Post_writable_byte_size_(size) _VCRT_ALLOCATOR void* __CRTDECL
    operator new[](std::size_t size, std::align_val_t align,
                   const std::nothrow_t&) noexcept {
  if (size == 0) {
    size = 1;
  }

  return _aligned_malloc(size, static_cast<std::size_t>(align));
}
#else
void* operator new(std::size_t size, std::align_val_t align) {
  if (size == 0) {
    size = 1;
  }

  if (void* ptr{std::aligned_alloc(static_cast<std::size_t>(align), size)}) {
    return ptr;
  }

  throw std::bad_alloc();
}

void* operator new[](std::size_t size, std::align_val_t align) {
  if (size == 0) {
    size = 1;
  }

  if (void* ptr{std::aligned_alloc(static_cast<std::size_t>(align), size)}) {
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

  return std::aligned_alloc(static_cast<std::size_t>(align), size);
}

void* operator new[](std::size_t size, std::align_val_t align,
                     const std::nothrow_t&) noexcept {
  if (size == 0) {
    size = 1;
  }

  return std::aligned_alloc(static_cast<std::size_t>(align), size);
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
#endif  // __cpp_aligned_new

#endif  // COMET_TRACK_ALLOCATIONS
