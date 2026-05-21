// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/job/job.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/fiber/fiber_context.h"

namespace comet {
namespace job {
CounterWaiter::CounterWaiter(Counter& counter) : counter_{counter} {
  // Case: blockable thread (no fiber is being executed).
  if (!fiber::IsFiber()) {
    while (!counter.IsZero());
    return;
  }

  while (!counter.IsZero()) {
    fiber::Yield();
  }
}

void Counter::Reset() { value_ = 0; }

void Counter::Increment() { ++value_; }

void Counter::Decrement() {
  COMET_ASSERT(!IsZero(), "job::Counter::Decrement",
               "counter cannot be decremented", "value", value_);
  --value_;
}

bool Counter::IsZero() const noexcept { return value_ == 0; }

usize Counter::GetValue() const noexcept { return value_; }
}  // namespace job
}  // namespace comet