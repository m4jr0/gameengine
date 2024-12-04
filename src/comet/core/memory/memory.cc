// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "memory.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#include "comet/core/type/array.h"

#ifdef COMET_MSVC
#include "comet/core/windows.h"
#else
#include <sys/sysinfo.h>
#include <unistd.h>
#endif  // COMET_MSVC

#include "comet/core/c_string.h"

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
    case kEngineMemoryTagTaggedHeap:
      return "tagged_heap";
    case kEngineMemoryTagStringId:
      return "string_id";
    case kEngineMemoryTagFrame0:
      return "frame_0";
    case kEngineMemoryTagFrame1:
      return "frame_1";
    case kEngineMemoryTagFrame2:
      return "frame_2";
    case kEngineMemoryTagDoubleFrame0:
      return "double_frame_0";
    case kEngineMemoryTagDoubleFrame1:
      return "double_frame_1";
    case kEngineMemoryTagDoubleFrame2:
      return "double_frame_2";
    case kEngineMemoryTagRendering:
      return "rendering";
    case kEngineMemoryTagRenderingInternal:
      return "rendering_internal";
    case kEngineMemoryTagRenderingDevice:
      return "rendering_device (VRAM)";
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

void ClearMemory(void* ptr, usize size) {
  auto* bytes{static_cast<u8*>(ptr)};

  for (usize i{0}; i < size; ++i) {
    bytes[i] = 0;
  }
}

void* StoreShiftAndReturnAligned(u8* ptr, [[maybe_unused]] usize data_size,
                                 [[maybe_unused]] usize allocation_size,
                                 Alignment align) {
  COMET_ASSERT(allocation_size > data_size,
               "Cannot save shift, allocation size is too small!");
  auto* aligned_ptr{AlignPointer(ptr, align)};

  // Case: pointer is already aligned. We have a minimal shift of 1 byte, so we
  // move the pointer to "align" bytes as a convention.
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
}  // namespace memory
}  // namespace comet
