// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "core/time/time_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_string.h"

namespace comet {
namespace time {
namespace internal {
static void AppendPadded2(usize value, schar* buffer, usize buffer_len,
                          usize& offset) {
  COMET_ASSERT(buffer != nullptr, "time::AppendPadded2", "buffer is null");
  COMET_ASSERT(offset + 2 < buffer_len, "time::AppendPadded2",
               "buffer is too small");

  if (value < 10) {
    buffer[offset++] = '0';
  }

  usize len{0};
  ConvertToStr(static_cast<u32>(value), buffer + offset, buffer_len - offset,
               &len);
  offset += len;
}
}  // namespace internal

void GetTimeString(f64 time, schar* buffer, usize buffer_len) {
  COMET_ASSERT(buffer != nullptr, "time::GetTimeString", "buffer is null");
  COMET_ASSERT(buffer_len >= 16, "time::GetTimeString", "buffer is too small");

  const auto total_seconds{static_cast<usize>(time < .0 ? .0 : time)};
  const auto hours{total_seconds / 3600};
  const auto minutes{(total_seconds / 60) % 60};
  const auto seconds{total_seconds % 60};

  usize offset{0};
  internal::AppendPadded2(hours, buffer, buffer_len, offset);
  buffer[offset++] = ':';
  internal::AppendPadded2(minutes, buffer, buffer_len, offset);
  buffer[offset++] = ':';
  internal::AppendPadded2(seconds, buffer, buffer_len, offset);

  COMET_ASSERT(offset < buffer_len, "time::GetTimeString",
               "buffer is too small");
  buffer[offset] = '\0';
}
}  // namespace time
}  // namespace comet
