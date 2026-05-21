// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_TYPE_TRAITS_H_
#define COMET_CORE_TYPE_TRAITS_H_

#include "comet/core/essentials.h"

namespace comet {
template <typename T, typename... Rest>
struct EnforceSame {
  static_assert(std::conjunction_v<std::is_same<T, Rest>...>,
                "all elements must have the same type");
  using type = T;
};

template <typename>
inline constexpr bool always_false_v = false;

template <typename TEnum>
constexpr auto ToUnderlying(TEnum value) {
  return static_cast<std::underlying_type_t<TEnum>>(value);
}
}  // namespace comet

#endif  // COMET_CORE_TYPE_TRAITS_H_
