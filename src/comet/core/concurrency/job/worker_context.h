// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONCURRENCY_JOB_WORKER_CONTEXT_H_
#define COMET_COMET_CORE_CONCURRENCY_JOB_WORKER_CONTEXT_H_

#include "comet/core/concurrency/job/worker.h"
#include "comet/core/essentials.h"

namespace comet {
namespace job {
namespace internal {
static_assert(std::atomic<usize>::is_always_lock_free,
              "std::atomic<usize> needs to be always lock-free. Unsupported "
              "architecture");
inline static std::atomic<usize> active_worker_count{0};
inline static std::atomic<usize> active_fiber_worker_count{0};
inline static std::atomic<usize> active_io_worker_count{0};

void AttachWorker(Worker* worker);
void DetachWorker();
void AttachFiberWorker(FiberWorker*);
void DetachFiberWorker();
void AttachIOWorker(IOWorker*);
void DetachIOWorker();
}  // namespace internal

bool IsWorkerAttached();
bool IsFiberWorker();
bool IsIOWorker();
WorkerId GetWorkerId();
WorkerId GetWorkerTypeIndex();
WorkerTag GetWorkerTag();
Worker& GetWorker();
usize GetCurrentWorkerCount();
usize GetCurrentFiberWorkerCount();
usize GetCurrentIOWorkerCount();
bool IsMainThreadWorkerDisabled();
}  // namespace job
}  // namespace comet

#endif  // COMET_COMET_CORE_CONCURRENCY_JOB_WORKER_CONTEXT_H_