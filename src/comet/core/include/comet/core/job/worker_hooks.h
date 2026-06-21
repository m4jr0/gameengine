// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_JOB_WORKER_HOOKS_H_
#define COMET_CORE_JOB_WORKER_HOOKS_H_

#include "comet/core/essentials.h"

namespace comet {
namespace job {
using WorkerLifecycleCallback = void (*)();

struct WorkerLifecycleCallbacks {
  WorkerLifecycleCallback attach{nullptr};
  WorkerLifecycleCallback detach{nullptr};

  WorkerLifecycleCallback fiber_attach{nullptr};
  WorkerLifecycleCallback fiber_detach{nullptr};

  WorkerLifecycleCallback io_attach{nullptr};
  WorkerLifecycleCallback io_detach{nullptr};
};
}  // namespace job
}  // namespace comet

#endif  // COMET_CORE_JOB_WORKER_HOOKS_H_