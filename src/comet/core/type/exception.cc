// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "exception.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_string.h"
#include "comet/core/generator.h"
#include "comet/math/math_common.h"

namespace comet {
const schar* comet::MaximumCapacityReachedError::GenerateTmpErrorMessage(
    usize capacity) {
  constexpr usize kMinBufferSize{128};
  const schar* kText{"Structure instance is full ("};
  auto text_len{GetLength(kText)};
  // Add 10 for null terminator, ) and some room for the capacity.
  auto buffer_len{math::Max(GetLength(kText) + 10, kMinBufferSize)};
  auto* tmp{GenerateForOneFrame<schar>(text_len)};
  Copy(tmp, kText, text_len);
  usize offset;
  ConvertToStr(capacity, tmp + text_len, buffer_len - text_len, &offset);
  text_len += offset;

  if (buffer_len - text_len > 1) {
    tmp[text_len++] = ')';
    tmp[text_len] = '\0';
  }

  return tmp;
}
}  // namespace comet