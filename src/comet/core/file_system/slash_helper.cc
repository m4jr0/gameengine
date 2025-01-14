// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "slash_helper.h"

namespace comet {
bool IsSlash(tchar c) {
#ifdef COMET_MSVC
  return c == COMET_TCHAR('/') || c == COMET_TCHAR('\\');
#else
  return c == COMET_TCHAR('/');
#endif  // COMET_MSVC
}
}  // namespace comet
