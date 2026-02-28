// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_GENERATOR_H_
#define COMET_COMET_CORE_GENERATOR_H_

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_allocator.h"

namespace comet {
template <typename TChar>
TChar* GenerateForOneFrame(usize length) {
  COMET_ASSERT(length > 0, "Cannot allocate temporary string of length 0!");
  // Add 1 for null terminator.
  auto* new_str{reinterpret_cast<TChar*>(
      COMET_FRAME_ALLOC_ALIGNED(sizeof(TChar) * (length + 1), alignof(TChar)))};
  new_str[0] = COMET_TCHAR('\0');
  return new_str;
}

template <typename ReturnedTChar, typename TCharParam>
ReturnedTChar* GenerateForOneFrame(const TCharParam* str, usize length) {
  COMET_ASSERT(str != nullptr, "String provided is null!");
  COMET_ASSERT(length > 0, "Cannot allocate temporary string of length 0!");
  // Add 1 for null terminator.
  auto* new_str{reinterpret_cast<ReturnedTChar*>(COMET_FRAME_ALLOC_ALIGNED(
      sizeof(ReturnedTChar) * (length + 1), alignof(ReturnedTChar)))};
  Copy(new_str, str, length);
  new_str[length] = COMET_TCHAR('\0');
  return new_str;
}

template <typename TChar>
TChar* GenerateForTwoFrames(usize length) {
  COMET_ASSERT(length > 0, "Cannot allocate temporary string of length 0!");
  // Add 1 for null terminator.
  auto* new_str{reinterpret_cast<TChar*>(COMET_DOUBLE_FRAME_ALLOC_ALIGNED(
      sizeof(TChar) * (length + 1), alignof(TChar)))};
  new_str[0] = COMET_TCHAR('\0');
  return new_str;
}

template <typename ReturnedTChar, typename TCharParam>
ReturnedTChar* GenerateForTwoFrames(const TCharParam* str, usize length) {
  COMET_ASSERT(str != nullptr, "String provided is null!");
  COMET_ASSERT(length > 0, "Cannot allocate temporary string of length 0!");
  // Add 1 for null terminator.
  auto* new_str{
      reinterpret_cast<ReturnedTChar*>(COMET_DOUBLE_FRAME_ALLOC_ALIGNED(
          sizeof(ReturnedTChar) * (length + 1), alignof(ReturnedTChar)))};
  Copy(new_str, str, length);
  new_str[length] = COMET_TCHAR('\0');
  return new_str;
}
}  // namespace comet

#endif  // COMET_COMET_CORE_GENERATOR_H_
