// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_FILE_SYSTEM_H_
#define COMET_COMET_CORE_FILE_SYSTEM_H_

#include "comet/core/essentials.h"

namespace comet {
constexpr auto kNativeSlash{
#ifdef COMET_WINDOWS
#ifdef COMET_NORMALIZE_PATHS
    COMET_TCHAR('/')
#else
    COMET_TCHAR('\\')
#endif  // COMET_NORMALIZE_PATHS
#else
    COMET_TCHAR('/')
#endif  // COMET_WINDOWS
};

bool IsSlash(tchar c);
}  // namespace comet

#endif  // COMET_COMET_CORE_FILE_SYSTEM_H_
