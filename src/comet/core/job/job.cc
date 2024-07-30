// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "job.h"

#include "comet/core/debug.h"
#include "comet/core/job/fiber/fiber_context.h"

namespace comet {
namespace job {
CounterGuard::CounterGuard(Counter& counter) : counter_{counter} {
  // Case: blockable thread (no fiber is being executed).
  if (!IsFiber()) {
    while (!counter.IsZero())
      ;
    return;
  }

  while (!counter.IsZero()) {
    Yield();
  }
}

void Counter::Reset() { value_ = 0; }

void Counter::Increment() { ++value_; }

void Counter::Decrement() {
  COMET_ASSERT(!IsZero(), "Counter cannot be decremented! Value is 0!");
  --value_;
}

bool Counter::IsZero() const noexcept { return value_ == 0; }

uindex Counter::GetValue() const noexcept { return value_; }
}  // namespace job
}  // namespace comet