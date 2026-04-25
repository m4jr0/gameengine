// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONCURRENCY_JOB_JOB_LABEL_H_
#define COMET_COMET_CORE_CONCURRENCY_JOB_JOB_LABEL_H_

#include "comet/core/concurrency/job/job.h"
#include "comet/core/essentials.h"

namespace comet {
namespace job {
class CounterFence {
 public:
  explicit CounterFence(job::Counter* counter);
  CounterFence(const CounterFence&) = default;
  CounterFence(CounterFence&&) noexcept = default;
  CounterFence& operator=(const CounterFence&) = default;
  CounterFence& operator=(CounterFence&&) noexcept = default;
  ~CounterFence() = default;

  bool Poll() const noexcept;

 private:
  job::Counter* counter_{nullptr};
};
}  // namespace job
}  // namespace comet

#endif  // COMET_COMET_CORE_CONCURRENCY_JOB_JOB_LABEL_H_