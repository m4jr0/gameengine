// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_core_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/file_system/file_system_label.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug/debug_label.h"

namespace comet {
const schar* GetRootTypeLabel(RootType root_type) {
  switch (root_type) {
    case RootType::Unknown:
      return "unknown";
    case RootType::Unix:
      return "unix";
    case RootType::WindowsDriveLetter:
      return "windows_drive_letter";
    case RootType::WindowsExtended:
      return "windows_extended";
    case RootType::WindowsUnc:
      return "windows_unc";
    case RootType::Invalid:
      return "invalid";
  }

  return kUnknownLabel;
}
}  // namespace comet
