// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_DEBUG_LABEL_H_
#define COMET_COMET_CORE_DEBUG_LABEL_H_

#include "comet/core/c_string.h"

namespace comet {
constexpr const schar* kUnknownLabel{"<?>"};
constexpr auto kUnknownLabelLen{GetLength(kUnknownLabel)};
}  // namespace comet

#endif  // COMET_COMET_CORE_DEBUG_LABEL_H_