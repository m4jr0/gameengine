// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "memory_utils.h"

#include <immintrin.h>

#include <cstdint>
#include <cstdio>
#include <cstring>

#include "comet/core/memory/allocation_tracking.h"
#include "comet/core/type/array.h"

#ifdef COMET_MSVC
#include "comet/core/windows.h"
#else
#include <sys/sysinfo.h>
#include <unistd.h>
#endif  // COMET_MSVC

#include "comet/core/c_string.h"
#include "comet/core/processor.h"

#ifdef COMET_INVESTIGATE_MEMORY_CORRUPTION
#include <atomic>
#endif  // COMET_INVESTIGATE_MEMORY_CORRUPTION

namespace comet {
namespace memory {
#ifdef COMET_ALLOW_CUSTOM_MEMORY_TAG_LABELS
namespace internal {
GetCustomMemoryTagLabelFunc get_custom_memory_tag_label_func{nullptr};
}  // namespace internal

void AttachGetCustomMemoryTagLabelFunc(GetCustomMemoryTagLabelFunc func) {
  COMET_ASSERT(func != nullptr,
               "GetCustomMemoryTagLabelFunc provided is null!");
  internal::get_custom_memory_tag_label_func = func;
}

void DetachGetCustomMemoryTagLabelFunc() {
  internal::get_custom_memory_tag_label_func = nullptr;
}
#endif  // COMET_ALLOW_CUSTOM_MEMORY_TAG_LABELS

const schar* GetMemoryTagLabel(MemoryTag tag) {
  switch (tag) {
    case kEngineMemoryTagUntagged:
      return "untagged";
    case kEngineMemoryTagConfig:
      return "config";
    case kEngineMemoryTagTaggedHeap:
      return "tagged_heap";
    case kEngineMemoryTagGid:
      return "gid";
    case kEngineMemoryTagStringId:
      return "string_id";
    case kEngineMemoryTagFrame:
      return "frame";
    case kEngineMemoryTagInput:
      return "input";
    case kEngineMemoryTagFrameExtended:
      return "frame_extended";
    case kEngineMemoryTagDoubleFrame:
      return "double_frame";
    case kEngineMemoryTagDoubleFrameExtended1:
      return "double_frame_extended_1";
    case kEngineMemoryTagDoubleFrameExtended2:
      return "double_frame_extended_2";
    case kEngineMemoryTagGeometry:
      return "geometry";
    case kEngineMemoryTagRendering:
      return "rendering";
    case kEngineMemoryTagRenderingInternal:
      return "rendering_internal";
    case kEngineMemoryTagRenderingDevice:
      return "rendering_device (VRAM)";
    case kEngineMemoryTagResource:
      return "resource";
    case kEngineMemoryTagTString:
      return "tstring";
    case kEngineMemoryTagEntity:
      return "entity";
    case kEngineMemoryTagFiber:
      return "fiber";
    case kEngineMemoryTagThreadProvider:
      return "thread_provider";
    case kEngineMemoryTagEvent:
      return "event";
    case kEngineMemoryTagDebug:
      return "debug";
    case kEngineMemoryTagUserBase:
      return "user_base (should not be used)";
    case kEngineMemoryTagInvalid:
      return "invalid (please investigate)";
#ifdef COMET_ALLOW_CUSTOM_MEMORY_TAG_LABELS
    default:
      if (internal::get_custom_memory_tag_label_func != nullptr) {
        const auto* result{internal::get_custom_memory_tag_label_func(tag)};

        if (result != nullptr) {
          return result;
        }
      }
#endif  // COMET_ALLOW_CUSTOM_MEMORY_TAG_LABELS
  }

  return "???";
}

void* CopyMemory(void* dst, const void* src, usize size) {
  return std::memcpy(dst, src, size);
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

void* StoreShiftAndReturnAligned(u8* ptr, [[maybe_unused]] usize data_size,
                                 [[maybe_unused]] usize allocation_size,
                                 Alignment align) {
  COMET_ASSERT(allocation_size > data_size,
               "Cannot save shift, allocation size is too small!");
  auto* aligned_ptr{AlignPointer(ptr, align)};

  // Case: pointer is already aligned. We have a minimal shift of 1 byte, so
  // we move the pointer to "align" bytes as a convention.
  if (aligned_ptr == ptr) {
    aligned_ptr += align;
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
  COMET_ASSERT(buffer_len > 2, "Buffer provided is too small!");

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

  COMET_ASSERT(l < kUnits.GetSize(), "Size is too big: ", size, "!");
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
  COMET_ASSERT(ptr != nullptr, "Pointer provided is null!");
  COMET_ASSERT(size != 0, "Size provided is 0!");
  constexpr StaticArray<u8, 4> kPoison{0xde, 0xad, 0xbe, 0xef};
  constexpr auto kPoisonLen{kPoison.GetSize()};

#ifdef COMET_INVESTIGATE_MEMORY_CORRUPTION
  static_assert(std::atomic<u64>::is_always_lock_free,
                "std::atomic<u64> needs to be always lock-free. Unsupported "
                "architecture");
  static std::atomic<u64> allocation_id_counter{0};
  auto allocation_id{
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

  [[maybe_unused]] auto is_ok{
      GetPhysicallyInstalledSystemMemory(&total_memory_in_kilobytes)};
  COMET_ASSERT(is_ok, "Could not retrieve the total memory from the system!");

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

void* Allocate(usize size, MemoryTag tag) {
  return AllocateAligned(size, 1, tag);
}

void* AllocateAligned(usize size, Alignment align,
                      [[maybe_unused]] MemoryTag tag) {
  // TODO(m4jr0): Handle memory tag.
  usize allocation_size{size + align};
  auto* ptr{new u8[allocation_size]};

  if (ptr == nullptr) {
    COMET_ASSERT(false, "Unable to allocate with size ", size, ", align ",
                 align, " and tag ", GetMemoryTagLabel(tag), "!");
    throw std::bad_alloc();
  }

  COMET_REGISTER_PLATFORM_ALLOCATION(ptr, allocation_size, tag);
  COMET_POISON(ptr, allocation_size);
  return StoreShiftAndReturnAligned(ptr, size, allocation_size, align);
}

void Deallocate(void* ptr) {
  COMET_REGISTER_PLATFORM_DEALLOCATION(ptr);
  delete[] static_cast<u8*>(ResolveNonAligned(ptr));
}
}  // namespace memory
}  // namespace comet
