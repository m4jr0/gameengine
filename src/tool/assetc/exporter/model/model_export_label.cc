// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "assetc_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "model_export_label.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace tool {
namespace assetc {
const schar* GetModelExportTypeLabel(ModelExportType type) {
  switch (type) {
    case ModelExportType::Unknown:
      return "unknown";
    case ModelExportType::Static:
      return "static";
    case ModelExportType::Skeletal:
      return "skeletal";
    default:
      return kUnknownLabel;
  }
}
}  // namespace assetc
}  // namespace tool
}  // namespace comet