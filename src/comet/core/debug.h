// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_DEBUG_H_
#define COMET_COMET_CORE_DEBUG_H_

#ifndef NDEBUG
#define COMET_DEBUG
#endif  // !NDEBUG

namespace comet {
namespace debug {
#ifndef COMET_DEBUG
#define COMET_ASSERT(assertion, ...)
#else
#define COMET_ASSERT(assertion, ...)               \
  do {                                             \
    const auto isOk{static_cast<bool>(assertion)}; \
                                                   \
    if (!isOk) {                                   \
      COMET_LOG_GLOBAL_ERROR(__VA_ARGS__);         \
    }                                              \
                                                   \
    assert(isOk);                                  \
  } while (0)
#endif
}  // namespace debug
}  // namespace comet

#endif  // COMET_COMET_CORE_DEBUG_H_
