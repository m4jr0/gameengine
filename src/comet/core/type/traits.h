// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_TRAITS_H_
#define COMET_COMET_CORE_TYPE_TRAITS_H_

#include "comet/core/essentials.h"

namespace comet {
template <typename>
inline constexpr bool always_false_v = false;
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_TRAITS_H_