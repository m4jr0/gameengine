// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_ENGINE_THREADING_MAIN_THREAD_WORKER_POLICY_H_
#define COMET_ENGINE_THREADING_MAIN_THREAD_WORKER_POLICY_H_

#include "comet/core/job/job.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/memory.h"
#include "comet/runtime/memory/memory_tag.h"
#include "comet/runtime/event/event.h"
#include "comet/runtime/event/event_manager.h"

namespace comet {
namespace engine {
bool IsMainThreadWorkerDisabled();
}  // namespace engine
}  // namespace comet

#endif  // COMET_ENGINE_THREADING_MAIN_THREAD_WORKER_POLICY_H_
