// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_CONTINUATION_CONTINUATION_PHASE_H_
#define COMET_RUNTIME_CONTINUATION_CONTINUATION_PHASE_H_

#include "comet/core/essentials.h"

namespace comet {
enum class ContinuationPhase : u8 {
  BeginFrame,
  BeforeLogic,
  AfterLogic,
  BeforeEndFrame,
  AfterEndFrame,
};
}  // namespace comet

#endif  // COMET_RUNTIME_CONTINUATION_CONTINUATION_PHASE_H_