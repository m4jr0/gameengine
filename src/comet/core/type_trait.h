// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_TRAIT_H_
#define COMET_COMET_CORE_TYPE_TRAIT_H_

#include "comet/core/essentials.h"

namespace comet {
template <typename TEnum>
constexpr auto ToUnderlying(TEnum value) {
  return static_cast<std::underlying_type_t<TEnum>>(value);
}
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_TRAIT_H_
