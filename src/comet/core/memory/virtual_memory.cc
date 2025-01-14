// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "virtual_memory.h"

#ifdef COMET_WINDOWS
#include "comet/core/windows.h"
#else
#include <sys/mman.h>
#endif  // COMET_WINDOWS

namespace comet {
namespace memory {
void* ReserveVirtualMemory(usize size) {
#ifdef COMET_WINDOWS
  auto* memory{VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_NOACCESS)};
  COMET_ASSERT(memory != nullptr,
               "Failed to reserve memory! Error code: ", GetLastError());
#else
  auto* memory{
      mmap(nullptr, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)};
  COMET_ASSERT(memory != MAP_FAILED,
               "Failed to reserve memory! Error code: ", errno);
#endif  // COMET_WINDOWS

  return memory;
}

void* CommitVirtualMemory(void* memory, usize size) {
#ifdef COMET_WINDOWS
  auto* committed_memory{
      VirtualAlloc(memory, size, MEM_COMMIT, PAGE_READWRITE)};
  COMET_ASSERT(committed_memory != nullptr,
               "Failed to commit memory! Error code: ", GetLastError());
  return committed_memory;
#else
  [[maybe_unused]] auto result{mprotect(memory, size, PROT_READ | PROT_WRITE)};
  COMET_ASSERT(result == 0, "Failed to commit memory! Error code: ", errno);
  return memory;
#endif  // COMET_WINDOWS
}

void FreeVirtualMemory(void* memory, [[maybe_unused]] usize size) {
#ifdef COMET_WINDOWS
  [[maybe_unused]] auto is_ok{VirtualFree(memory, 0, MEM_RELEASE)};
  COMET_ASSERT(is_ok, "Failed to release memory! Error code: ", GetLastError());
#else
  [[maybe_unused]] auto result{munmap(memory, size)};
  COMET_ASSERT(result == 0, "Failed to release memory! Error code: ", errno);
#endif  // COMET_WINDOWS
}
}  // namespace memory
}  // namespace comet
