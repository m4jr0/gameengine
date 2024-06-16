// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_JOB_FIBER_PRIMITIVE_H_
#define COMET_COMET_CORE_JOB_FIBER_PRIMITIVE_H_

#include "comet/core/job/fiber/fiber.h"
#include "comet_precompile.h"

namespace comet {
namespace job {
class FiberMutex {
 public:
  FiberMutex() = default;
  FiberMutex(const FiberMutex&) = delete;
  FiberMutex(FiberMutex&&) = delete;
  FiberMutex& operator=(const FiberMutex&) = delete;
  FiberMutex& operator=(FiberMutex&&) = delete;
  virtual ~FiberMutex() = default;

  void Lock();
  void Unlock();

 private:
  std::atomic<bool> is_locked_{false};
  std::deque<Fiber*> awaiting_fibers_{};
};

class FiberCV {
 public:
  FiberCV() = default;
  FiberCV(const FiberCV&) = delete;
  FiberCV(FiberCV&&) = delete;
  FiberCV& operator=(const FiberCV&) = delete;
  FiberCV& operator=(FiberCV&&) = delete;
  virtual ~FiberCV() = default;

  void Wait(FiberMutex& mtx);

  template <typename Predicate>
  void Wait(FiberMutex& mutex, Predicate&& pred) {
    if (pred()) {
      return;
    }

    auto* fiber{GetCurrentFiber()}; // >:3 Wrong function.
    COMET_ASSERT(fiber != nullptr, "Fiber is null!");
    mutex.Lock();
  }

  void NotifyOne();
  void NotifyAll();

 private:
  std::atomic<uindex> waiting_count_{0};
  std::atomic<uindex> waiting_on_pred_count_{0};
  std::deque<Fiber*> awaiting_fibers_{};
};
}  // namespace job
}  // namespace comet

#endif  // COMET_COMET_CORE_JOB_FIBER_PRIMITIVE_H_