// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_core_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/memory/memory_utils.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <immintrin.h>

#include <cstdint>
#include <cstdio>
#include <cstring>

#ifndef COMET_MSVC
#include <sys/sysinfo.h>
#include <unistd.h>
#endif  // !COMET_MSVC

#ifdef COMET_INVESTIGATE_MEMORY_CORRUPTION
#include <atomic>
#endif  // COMET_INVESTIGATE_MEMORY_CORRUPTION
////////////////////////////////////////////////////////////////////////////////

#include "comet/runtime/memory/allocation_tracking.h"
#include "comet/core/container/array.h"

#ifdef COMET_MSVC
#include "comet/core/windows.h"
#endif  // COMET_MSVC

#include "comet/core/string/c_string.h"
#include "comet/runtime/memory/memory_label.h"
#include "comet/core/processor/processor.h"

namespace comet {
namespace memory {
void* CopyMemory(void* dst, const void* src, usize size) {
  return std::memcpy(dst, src, size);
}

void* MoveMemory(void* dst, const void* src, usize size) {
  return std::memmove(dst, src, size);
}

void* CopyOrMoveMemory(void* dst, const void* src, usize size) {
  const auto* s{static_cast<const u8*>(src)};
  auto* d{static_cast<u8*>(dst)};

  if (d < s + size && s < d + size) {
    return MoveMemory(dst, src, size);
  }

  return CopyMemory(dst, src, size);
}

void Memset(void* ptr, u8 value, usize size) {
  // std::memset casts the value to an unsigned char anyway, so taking a value
  // as a u8 is OK.
  std::memset(ptr, static_cast<int>(value), size);
}

void AVXMemset(void* ptr, u8 value, usize size) {
  auto* cur{static_cast<u8*>(ptr)};
  auto* end{cur + size};

  __m256i avx_value{_mm256_set1_epi8(static_cast<char>(value))};

  while (cur + 32 <= end) {
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(cur), avx_value);
    cur += 32;
  }

  while (cur < end) {
    *cur++ = value;
  }
}

void FastMemset(void* ptr, u8 value, usize size) {
  if (IsAVXSupported()) {
    AVXMemset(ptr, value, size);
  } else {
    Memset(ptr, value, size);
  }
}

void ClearMemory(void* ptr, usize size) { FastMemset(ptr, 0, size); }

void* StoreShiftAndReturnAligned(void* ptr, [[maybe_unused]] usize data_size,
                                 [[maybe_unused]] usize allocation_size,
                                 Alignment align) {
  COMET_ASSERT(allocation_size > data_size,
               "memory_utils::StoreShiftAndReturnAligned",
               "allocation size is too small to store shift", "allocation_size",
               allocation_size, "data_size", data_size);

  const auto cast_ptr{static_cast<u8*>(ptr)};
  auto* aligned_ptr{AlignPointer(cast_ptr, align)};

  // Case: pointer is already aligned. We have a minimal shift of 1 byte, so
  // we move the pointer to "align" bytes as a convention.
  if (aligned_ptr == cast_ptr) {
    aligned_ptr += align;
  }

  const auto shift{aligned_ptr - cast_ptr};

  COMET_ASSERT(shift > 0 && shift <= kMaxAlignment,
               "memory_utils::StoreShiftAndReturnAligned",
               "alignment shift is invalid", "shift", shift, "max_alignment",
               kMaxAlignment);

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

void* ResolveNonAligned(void* ptr) {
  auto* aligned_ptr{reinterpret_cast<u8*>(ptr)};
  auto shift{static_cast<u16>(aligned_ptr[-1])};

  // Case: alignment is a power of 2, so using 1 byte to store it makes it a
  // maximum of a 128-byte alignment. So as a convention, a 0-byte shift
  // represents a 256-byte alignment to increase the limit to one more bit.
  if (shift == 0) {
    shift = kMaxAlignment;
  }

  return aligned_ptr - shift;
}

void GetMemorySizeString(ssize size, schar* buffer, usize buffer_len,
                         usize* out_len) {
  COMET_ASSERT(buffer_len > 2, "memory_utils::GetMemorySizeString",
               "buffer is too small", "buffer_len", buffer_len);

  if (size < 0) {
    buffer[0] = '-';
    ++buffer;
    --buffer_len;
    size = -size;
  }

  constexpr StaticArray kUnits{"bytes", "KiB", "MiB", "GiB", "TiB",
                               "PiB",   "EiB", "ZiB", "YiB"};
  usize l{0};
  auto n{static_cast<f64>(size)};

  while (n >= 1024 && ++l) {
    n = n / 1024;
  }

  COMET_ASSERT(l < kUnits.GetSize(), "memory_utils::GetMemorySizeString",
               "size unit index is out of bounds", "size", size, "unit_index",
               l, "unit_count", kUnits.GetSize());

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

#ifdef COMET_POISON_ALLOCATIONS
void Poison(void* ptr, usize size) {
  COMET_ASSERT(ptr != nullptr, "memory_utils::Poison", "pointer is null");
  COMET_ASSERT(size != 0, "memory_utils::Poison", "size is zero");

  constexpr StaticArray<u8, 4> kPoison{0xde, 0xad, 0xbe, 0xef};
  constexpr auto kPoisonLen{kPoison.GetSize()};

#ifdef COMET_INVESTIGATE_MEMORY_CORRUPTION
  static_assert(std::atomic<u64>::is_always_lock_free,
                "std::atomic<u64> must be always lock-free");
  static std::atomic<u64> allocation_id_counter{0};

  const auto allocation_id{
      allocation_id_counter.fetch_add(1, std::memory_order_acq_rel)};
  constexpr auto kAllocationIdSize{sizeof(allocation_id)};
#endif  // COMET_INVESTIGATE_MEMORY_CORRUPTION

  usize i{0};
  auto* cur{static_cast<u8*>(ptr)};
  auto* top{cur + size};

#ifdef COMET_INVESTIGATE_MEMORY_CORRUPTION
  while (cur + kAllocationIdSize <= top) {
    CopyMemory(cur, &allocation_id, kAllocationIdSize);
    cur += kAllocationIdSize;

    for (i = 0; i < kPoisonLen && cur < top; ++i) {
      *cur = kPoison[i];
      ++cur;
    }
  }
#endif  // COMET_INVESTIGATE_MEMORY_CORRUPTION

  while (cur < top) {
    *cur = kPoison[i % kPoisonLen];
    ++i;
    ++cur;
  }
}
#endif  //  COMET_POISON_ALLOCATIONS

MemoryDescr GetMemoryDescr() {
  MemoryDescr descr{};

#ifdef COMET_WINDOWS
  SYSTEM_INFO system_info;
  GetSystemInfo(&system_info);

  ULONGLONG total_memory_in_kilobytes{0};

  [[maybe_unused]] const auto is_ok{
      GetPhysicallyInstalledSystemMemory(&total_memory_in_kilobytes)};
  COMET_ASSERT(is_ok, "memory_utils::GetMemoryDescr",
               "could not retrieve total system memory");

  descr.total_memory_size =
      static_cast<usize>(total_memory_in_kilobytes * 1024);

  descr.page_size = system_info.dwPageSize;
  descr.large_page_size = GetLargePageMinimum();
#else
  struct sysinfo si;
  sysinfo(&si);

  descr.total_memory_size = static_cast<usize>(si.totalram);

  descr.page_size = sysconf(_SC_PAGESIZE);

#ifdef _SC_HUGEPAGESIZE
  descr.large_page_size = sysconf(_SC_HUGEPAGESIZE);
#else
  // Fallback: check /proc/meminfo to determine the huge page size.
  auto* meminfo{fopen("/proc/meminfo", "r")};

  if (meminfo != nullptr) {
    schar buffer[256];
    const schar* hugepagesize_key{"Hugepagesize:"};
    descr.large_page_size = 0;

    while (fgets(buffer, sizeof(buffer), meminfo) != nullptr) {
      if (strncmp(buffer, hugepagesize_key, strlen(hugepagesize_key)) == 0) {
        u64 size_kb{0};
        sscanf(buffer + strlen(hugepagesize_key), "%lu", &size_kb);
        descr.large_page_size = size_kb * 1024;
        break;
      }
    }

    fclose(meminfo);
  } else {
    descr.large_page_size =
        0;  // If /proc/meminfo can't be opened, fallback to 0
  }
#endif  // _SC_HUGEPAGESIZE
#endif  // COMET_WINDOWS

  return descr;
}

void ConvertAddressToHex(uptr address, schar* buffer, usize buffer_len) {
  COMET_ASSERT(buffer_len >= kHexAddressBufferLen,
               "memory_utils::ConvertAddressToHex", "buffer is too small",
               "buffer_len", buffer_len, "required_buffer_len",
               kHexAddressBufferLen);
#ifdef COMET_WINDOWS
  std::snprintf(buffer, buffer_len, "0x%016llx", address);
#else
  std::snprintf(buffer, buffer_len, "0x%016lx", address);
#endif  // COMET_WINDOWS
}

void ConvertAddressToHex(const void* address, schar* buffer, usize buffer_len) {
  ConvertAddressToHex(reinterpret_cast<uptr>(address), buffer, buffer_len);
}
}  // namespace memory
}  // namespace comet
