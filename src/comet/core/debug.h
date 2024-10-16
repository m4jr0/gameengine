// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_DEBUG_H_
#define COMET_COMET_CORE_DEBUG_H_

#include <stdio.h>

#include <cassert>

#include "comet/core/logger.h"

#if defined(_MSC_VER)
#if defined(_CPPRTTI)
#define COMET_RTTI
#endif  // _CPPRTTI
// _MSC_VER
#elif defined(__clang__)
#if __has_feature(cxx_rtti)
#define COMET_RTTI
#endif  // __has_feature(cxx_rtti)
// defined(__clang__)
#elif defined(__GNUG__)
#if defined(__GXX_RTTI)
#define COMET_RTTI
#endif  // __GXX_RTTI
#endif  // defined(__GNUG__)

namespace comet {
void HandleCriticalError();
}  // namespace comet

#ifndef COMET_DEBUG
#define COMET_ASSERT(assertion, ...)
#else
#define COMET_ASSERT(assertion, ...)                              \
  do {                                                            \
    const auto isOk{static_cast<bool>(assertion)};                \
                                                                  \
    if (!isOk) {                                                  \
      COMET_LOG_GLOBAL_ERROR("[CRITICAL FAILURE] ", __VA_ARGS__); \
      comet::HandleCriticalError();                               \
    }                                                             \
                                                                  \
    assert(isOk);                                                 \
  } while (false)
#endif

#endif  // COMET_COMET_CORE_DEBUG_H_
