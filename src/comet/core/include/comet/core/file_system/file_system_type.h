// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_FILE_SYSTEM_FILE_SYSTEM_TYPE_H_
#define COMET_CORE_FILE_SYSTEM_FILE_SYSTEM_TYPE_H_

#include "comet/core/essentials.h"

namespace comet {
enum class RootType {
  Unknown = 0,
  Unix,
  WindowsDriveLetter,
  WindowsExtended,
  WindowsUnc,
  Invalid
};
}  // namespace comet

#endif  // COMET_CORE_FILE_SYSTEM_FILE_SYSTEM_TYPE_H_
