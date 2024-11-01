// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_DEBUG_H_
#define COMET_COMET_CORE_DEBUG_H_

#include <stdio.h>

#include <cassert>
#include <iostream>

#include "comet/core/define.h"
#include "comet/core/os.h"
#include "comet/core/type/primitive.h"

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
namespace debug {
void HandleCriticalError();

namespace internal {
template <typename... Targs>
void PrintParams(Targs &&...args) {
  ((std::cerr << std::forward<Targs>(args) << ' '), ...);
}
}  // namespace internal

void GenerateStackTrace(schar *buffer, usize buffer_len);
}  // namespace debug
}  // namespace comet

#ifndef COMET_DEBUG
#define COMET_CASSERT(assertion, message)
#define COMET_ASSERT(assertion, ...)
#else
#define COMET_CASSERT(assertion, message) assert(assertion &&message)
#define COMET_ASSERT(assertion, ...)                    \
  do {                                                  \
    const auto isOk{static_cast<bool>(assertion)};      \
                                                        \
    if (!isOk) {                                        \
      std::cerr << "[CRITICAL FAILURE] ";               \
      comet::debug::internal::PrintParams(__VA_ARGS__); \
      std::cerr << '\n';                                \
      comet::debug::HandleCriticalError();              \
    }                                                   \
                                                        \
    COMET_CASSERT(isOk, "Critical failure!");           \
  } while (false)
#endif

#endif  // COMET_COMET_CORE_DEBUG_H_
