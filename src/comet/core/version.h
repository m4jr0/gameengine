// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_VERSION_H_
#define COMET_COMET_CORE_VERSION_H_

// External. ///////////////////////////////////////////////////////////////////
#include <string_view>
////////////////////////////////////////////////////////////////////////////////

#ifndef COMET_NAME
static_assert(false, "COMET_NAME preprocessor macro must be defined!");
#endif  // COMET_NAME

#ifndef COMET_VERSION_MAJOR
static_assert(false, "COMET_VERSION_MAJOR preprocessor macro must be defined!");
#endif  // COMET_VERSION_MAJOR

#ifndef COMET_VERSION_MINOR
static_assert(false, "COMET_VERSION_MINOR preprocessor macro must be defined!");
#endif  // COMET_VERSION_MINOR

#ifndef COMET_VERSION_PATCH
static_assert(false, "COMET_VERSION_PATCH preprocessor macro must be defined!");
#endif  // COMET_VERSION_PATCH

#include "comet/core/essentials.h"

using namespace std::literals;

namespace comet {
namespace version {
static constexpr std::string_view kCometName{COMET_NAME};

constexpr u16 kCometVersionMajor{COMET_VERSION_MAJOR};
constexpr u16 kCometVersionMinor{COMET_VERSION_MINOR};
constexpr u16 kCometVersionPatch{COMET_VERSION_PATCH};

#undef COMET_NAME
#undef COMET_VERSION_MAJOR
#undef COMET_VERSION_MINOR
#undef COMET_VERSION_PATCH

const schar* GetVersionStr();
}  // namespace version
}  // namespace comet

#endif  // COMET_COMET_CORE_VERSION_H_
