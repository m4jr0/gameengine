// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "job.h"
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
  COMET_ASSERT(!IsZero(), "Counter cannot be decremented! Value is 0!");
  --value_;
}

bool Counter::IsZero() const noexcept { return value_ == 0; }

usize Counter::GetValue() const noexcept { return value_; }

const schar* GetJobStackSizeLabel(JobStackSize stack_size) {
  switch (stack_size) {
    case JobStackSize::Unknown:
      return "unknown";
    case JobStackSize::Normal:
      return "normal";
    case JobStackSize::Large:
      return "large";
#ifdef COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
    case JobStackSize::ExternalLibrary:
      return "external library";
#endif  // COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
  }

  return "???";
}
}  // namespace job
}  // namespace comet