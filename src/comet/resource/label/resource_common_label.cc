// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "resource_common_label.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug_label.h"

namespace comet {
namespace resource {
const schar* GetCompressionModeLabel(CompressionMode mode) {
  switch (mode) {
    case CompressionMode::None:
      return "none";
    case CompressionMode::Lz4:
      return "lz4";
    default:
      return kUnknownLabel;
  }
}

const schar* GetResourceLifeSpanLabel(ResourceLifeSpan life_span) {
  switch (life_span) {
    case ResourceLifeSpan::Unknown:
      return "unknown";
    case ResourceLifeSpan::Manual:
      return "manual";
    case ResourceLifeSpan::Scene:
      return "scene";
    case ResourceLifeSpan::Global:
      return "global";
    case ResourceLifeSpan::Immortal:
      return "immortal";
    default:
      return kUnknownLabel;
  }
}
}  // namespace resource
}  // namespace comet