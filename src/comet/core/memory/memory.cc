// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "memory.h"

#include "comet/core/c_string.h"

namespace comet {
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
  }

  return "???";
}

void* CopyMemory(void* dst, const void* src, uindex size) {
  return std::memcpy(dst, src, size);
}

void* Allocate(uindex size, MemoryTag tag) {
  return AllocateAligned(size, 0, tag);
}

void* AllocateAligned(uindex size, u16 alignment, MemoryTag tag) {
  // TODO(m4jr0): Handle memory tag.
  uindex final_size{size + alignment};
  auto* ptr{new u8[final_size]};

  if (ptr == nullptr) {
    COMET_ASSERT(false, "Unable to allocate with size ", size, ", alignment ",
                 alignment, " and tag ", GetMemoryTagLabel(tag), "!");
    throw std::bad_alloc();
  }

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
  // Set shift to 0 if it equals kMaxAlignment.
  aligned_ptr[-1] = shift & (static_cast<u8>(kMaxAlignment - 1));
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

void GetMemorySizeString(uindex size, schar* buffer, uindex buffer_len,
                         uindex* out_len) {
  constexpr std::array kUnits{"bytes", "KiB", "MiB", "GiB", "TiB",
                              "PiB",   "EiB", "ZiB", "YiB"};
  uindex l{0};
  auto n{static_cast<f64>(size)};

  while (n >= 1024 && ++l) {
    n = n / 1024;
  }

  COMET_ASSERT(l < kUnits.size(), "Size is too big: ", size, "!");
  uindex len;
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
}  // namespace comet
