// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_VERSION_H_
#define COMET_COMET_CORE_VERSION_H_

#include <string>

#include "comet/core/type/primitive.h"

namespace comet {
namespace version {
constexpr char kCometName[]{"Comet Game Engine"};
constexpr u16 kCometVersionMajor{0};
constexpr u16 kCometVersionMinor{0};
constexpr u16 kCometVersionPatch{1};

std::string GetFormattedVersion();
}  // namespace version
}  // namespace comet

#endif  // COMET_COMET_CORE_VERSION_H_
