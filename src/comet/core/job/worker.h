// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_JOB_WORKER_H_
#define COMET_COMET_CORE_JOB_WORKER_H_

#include "comet_precompile.h"

#include "comet/core/job/fiber/fiber.h"

namespace comet {
namespace job {
struct Worker {
  std::thread thread{};
  Fiber* current_fiber{};
};
}  // namespace job
}  // namespace comet

#endif  // COMET_COMET_CORE_JOB_WORKER_H_