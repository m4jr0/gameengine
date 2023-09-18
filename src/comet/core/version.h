// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_VERSION_H_
#define COMET_COMET_CORE_VERSION_H_

#include <string_view>

#include "comet/core/type/primitive.h"

using namespace std::literals;

namespace comet {
namespace version {
static constexpr auto kCometName{"Comet Game Engine"sv};

constexpr u16 kCometVersionMajor{0};
constexpr u16 kCometVersionMinor{0};
constexpr u16 kCometVersionPatch{1};

constexpr auto kCometVersionStr{kCometVersionMajor + '.' + kCometVersionMinor +
                                '.' + kCometVersionPatch};
}  // namespace version
}  // namespace comet

#endif  // COMET_COMET_CORE_VERSION_H_
