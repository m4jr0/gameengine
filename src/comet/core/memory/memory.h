// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MEMORY_H_
#define COMET_COMET_CORE_MEMORY_H_

#include "comet_precompile.h"

namespace comet {
inline uptr AlignAddress(uptr address, uindex alignment) {
  const uindex mask{alignment - 1};
  COMET_ASSERT((alignment & mask) == 0, "Bad alignment provided for address ",
               reinterpret_cast<void*>(address), ": ", alignment,
               "! Must be a power of 2.");
  return (address + mask) & ~mask;
}
inline uindex AlignSize(uindex size, uindex alignment) {
  const uindex mask{alignment - 1};
  COMET_ASSERT((alignment & mask) == 0, "Bad alignment provided for size ",
               size, ": ", alignment, "! Must be a power of 2.");
  return (size + mask) & ~mask;
}

template <typename T>
inline T* AlignPointer(T* ptr, uindex alignment) {
  return reinterpret_cast<T*>(
      AlignAddress(reinterpret_cast<uptr>(ptr), alignment));
}

template <typename T>
T* AllocAligned(T** ptr, uindex size, uindex alignment) {
  uindex worst_case_size{size + alignment - 1};
  *ptr = reinterpret_cast<T*>(new u8[worst_case_size]);
  return AlignPointer(*ptr, alignment);
}

template <typename T>
inline bool IsAligned(T* ptr, uindex alignment) noexcept {
  auto tmp{reinterpret_cast<uptr>(ptr)};
  return tmp % alignment == 0;
}

std::string GetMemorySizeString(uindex size);
}  // namespace comet

#endif  // COMET_COMET_CORE_MEMORY_H_
