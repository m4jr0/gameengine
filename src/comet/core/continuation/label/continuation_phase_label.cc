// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "continuation_phase_label.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug_label.h"

namespace comet {
const schar* GetContinuationPhaseLabel(ContinuationPhase phase) {
  switch (phase) {
    case ContinuationPhase::BeginFrame:
      return "begin_frame";
    case ContinuationPhase::BeforeLogic:
      return "before_logic";
    case ContinuationPhase::AfterLogic:
      return "after_logic";
    case ContinuationPhase::BeforeEndFrame:
      return "before_end_frame";
    case ContinuationPhase::AfterEndFrame:
      return "after_end_frame";
    default:
      return kUnknownLabel;
  }
}
}  // namespace comet