// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/frame/frame_string.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <cstdarg>
////////////////////////////////////////////////////////////////////////////////

namespace comet {
const schar* GenerateTmpFromFormat(usize buffer_size, const schar* format, ...) {
  COMET_ASSERT(buffer_size > 0, "GenerateTmpFromFormat",
               "buffer size must be greater than zero");
  COMET_ASSERT(format != nullptr, "GenerateTmpFromFormat",
               "format is null");

  auto* buffer{GenerateFrameString<schar>(buffer_size)};

  va_list args;
  va_start(args, format);
  VFormatToBuffer(buffer, buffer_size, format, args);
  va_end(args);

  return buffer;
}
}  // namespace comet
