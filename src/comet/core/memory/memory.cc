// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "memory.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>

#ifdef COMET_MSVC
#include "comet/core/windows.h"
#else
#include <sys/sysinfo.h>
#include <unistd.h>
#endif  // COMET_MSVC

#include "comet/core/c_string.h"
#include "comet/core/concurrency/fiber/fiber_context.h"
#include "comet/core/concurrency/thread/thread_utils.h"
#include "comet/core/memory/memory.h"

namespace comet {
namespace memory {
const schar* GetMemoryTagLabel(MemoryTag tag) {
  switch (tag) {
    case MemoryTag::Untagged:
      return "untagged";
    case MemoryTag::StringId:
      return "string id";
    case MemoryTag::OneFrame:
      return "one frame";
    case MemoryTag::TwoFrames:
      return "two frames";
    case MemoryTag::TString:
      return "tstring";
    case MemoryTag::Entity:
      return "entity";
    case MemoryTag::Fiber:
      return "fiber";
    default:
      return "???";
  }
}

void* CopyMemory(void* dst, const void* src, usize size) {
  return std::memcpy(dst, src, size);
}

void* Allocate(usize size, MemoryTag tag) {
  return AllocateAligned(size, 1, tag);
}

void* AllocateAligned(usize size, u16 alignment,
                      [[maybe_unused]] MemoryTag tag) {
  // TODO(m4jr0): Handle memory tag.
  usize final_size{size + alignment};
  auto* ptr{new u8[final_size]};

  if (ptr == nullptr) {
    COMET_ASSERT(false, "Unable to allocate with size ", size, ", alignment ",
                 alignment, " and tag ", GetMemoryTagLabel(tag), "!");
    throw std::bad_alloc();
  }

  COMET_POISON(ptr, final_size);
  auto* aligned_ptr{AlignPointer(ptr, alignment)};

  // Case: pointer is already aligned. We have a minimal shift of 1 byte, so we
  // move the pointer to "alignment" bytes as a convention.
  if (aligned_ptr == ptr) {
    aligned_ptr += alignment;
  }

  auto shift{aligned_ptr - ptr};
  COMET_ASSERT(shift > 0 && shift <= kMaxAlignment,
               "Invalid shift in memory allocation! Shift is ", shift,
               ", but it must be between ", 0, " and ", kMaxAlignment, ".");

#ifdef COMET_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif  // COMET_GCC
  // Set shift to 0 if it equals kMaxAlignment.
  aligned_ptr[-1] = shift & (static_cast<u8>(kMaxAlignment - 1));
#ifdef COMET_GCC
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
#endif  // COMET_GCC
  return aligned_ptr;
}

void Deallocate(void* p) {
  auto* aligned_ptr{reinterpret_cast<u8*>(p)};
  auto shift{static_cast<u16>(aligned_ptr[-1])};

  // Case: alignment is a power of 2, so using 1 byte to store it makes it a
  // maximum of a 128-byte alignment. So as a convention, a 0-byte shift
  // represents a 256-byte alignment to increase the limit to one more bit.
  if (shift == 0) {
    shift = kMaxAlignment;
  }

  auto* ptr{aligned_ptr - shift};
  delete[] ptr;
}

void GetMemorySizeString(usize size, schar* buffer, usize buffer_len,
                         usize* out_len) {
  constexpr std::array kUnits{"bytes", "KiB", "MiB", "GiB", "TiB",
                              "PiB",   "EiB", "ZiB", "YiB"};
  usize l{0};
  auto n{static_cast<f64>(size)};

  while (n >= 1024 && ++l) {
    n = n / 1024;
  }

  COMET_ASSERT(l < kUnits.size(), "Size is too big: ", size, "!");
  usize len;
  ConvertToStr(n, n < 10 && l > 0 ? 1 : 0, buffer, buffer_len, &len);

  buffer[len++] = ' ';
  const auto units_size{GetLength(kUnits[l])};
  Copy(buffer, kUnits[l], units_size, len);
  len += units_size;
  buffer[len] = '\0';

  if (out_len != nullptr) {
    *out_len = len;
  }
}

void Poison(void* ptr, usize size) {
  COMET_ASSERT(ptr != nullptr, "Pointer provided is null!");
  COMET_ASSERT(size != 0, "Size provided is 0!");
  constexpr std::array<u8, 4> kPoison{0xde, 0xad, 0xbe, 0xef};
  constexpr auto kPoisonLen{kPoison.size()};

  usize i{0};
  auto* cur{static_cast<u8*>(ptr)};
  auto* top{cur + size};

  while (cur != top) {
    *cur = kPoison[i % kPoisonLen];
    ++i;
    ++cur;
  }
}

MemoryDescr GetMemoryDescr() {
  MemoryDescr descr{};

#ifdef COMET_WINDOWS
  SYSTEM_INFO system_info;
  GetSystemInfo(&system_info);

  ULONGLONG total_memory_in_kilobytes{0};

  [[maybe_unused]] auto is_ok{
      GetPhysicallyInstalledSystemMemory(&total_memory_in_kilobytes)};
  COMET_ASSERT(is_ok, "Could not retrieve the total memory from the system!");

  descr.total_memory_size =
      static_cast<usize>(total_memory_in_kilobytes * 1024);

  descr.page_size = system_info.dwPageSize;
#else
  struct sysinfo si;
  sysinfo(&si);

  descr.total_memory_size = static_cast<usize>(si.totalram);

  descr.page_size = sysconf(_SC_PAGESIZE);
#endif  // COMET_WINDOWS

  return descr;
}

void ConvertAddressToHex(uptr address, schar* buffer, usize buffer_len) {
  COMET_ASSERT(buffer_len > kHexAddressLength,
               "Insufficient buffer size provided: ", buffer_len, " < ",
               kHexAddressLength, "!");
#ifdef COMET_WINDOWS
  std::snprintf(buffer, buffer_len, "0x%016llx", address);
#else
  std::snprintf(buffer, buffer_len, "0x%016lx", address);
#endif  // COMET_WINDOWS
}

void ConvertAddressToHex(void* address, schar* buffer, usize buffer_len) {
  ConvertAddressToHex(reinterpret_cast<uptr>(address), buffer, buffer_len);
}
}  // namespace memory
}  // namespace comet
