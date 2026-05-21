// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/job/job_fence.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace job {
CounterFence::CounterFence(job::Counter* counter) : counter_{counter} {}

bool CounterFence::Poll() const noexcept {
  return counter_ == nullptr || counter_->IsZero();
}
}  // namespace job
}  // namespace comet