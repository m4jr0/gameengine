// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_core_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/job/worker_context.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/runtime/conf/conf_manager.h"
#include "comet/runtime/conf/config_defaults.h"
#include "comet/runtime/conf/config_keys.h"
#include "comet/runtime/conf/config_value.h"

namespace comet {
namespace job {
static thread_local Worker* tls_current_worker{nullptr};

namespace internal {
void AttachWorker(Worker* worker) {
  COMET_CASSERT(tls_current_worker == nullptr, "worker is already attached");
  tls_current_worker = worker;
  active_worker_count.fetch_add(1, std::memory_order_acq_rel);
}

void DetachWorker() {
  COMET_CASSERT(tls_current_worker != nullptr, "worker is not attached");
  tls_current_worker = nullptr;
  active_worker_count.fetch_sub(1, std::memory_order_acq_rel);
}

void AttachFiberWorker(FiberWorker*) {
  active_fiber_worker_count.fetch_add(1, std::memory_order_acq_rel);
}

void DetachFiberWorker() {
  active_fiber_worker_count.fetch_sub(1, std::memory_order_acq_rel);
}

void AttachIOWorker(IOWorker*) {
  active_io_worker_count.fetch_add(1, std::memory_order_acq_rel);
}

void DetachIOWorker() {
  active_io_worker_count.fetch_sub(1, std::memory_order_acq_rel);
}
}  // namespace internal

bool IsWorkerAttached() { return tls_current_worker != nullptr; }

bool IsFiberWorker() { return GetWorker().GetTag() == FiberWorker::kTag_; }

bool IsIOWorker() { return GetWorker().GetTag() == IOWorker::kTag_; }

WorkerId GetWorkerId() {
  return tls_current_worker != nullptr ? tls_current_worker->GetId()
                                       : kInvalidWorkerId;
}

WorkerId GetWorkerTypeIndex() {
  return tls_current_worker != nullptr ? tls_current_worker->GetTypeIndex()
                                       : kInvalidWorkerTypeIndex;
}

WorkerTag GetWorkerTag() {
  return tls_current_worker != nullptr ? tls_current_worker->GetTag()
                                       : kInvalidWorkerTag;
}

Worker& GetWorker() {
  COMET_ASSERT(tls_current_worker != nullptr, "job::GetWorker",
               "current worker is null");
  return *tls_current_worker;
}

usize GetCurrentWorkerCount() {
  return internal::active_worker_count.load(std::memory_order_acquire);
}

usize GetCurrentFiberWorkerCount() {
  return internal::active_fiber_worker_count.load(std::memory_order_acquire);
}

usize GetCurrentIOWorkerCount() {
  return internal::active_io_worker_count.load(std::memory_order_acquire);
}
}  // namespace job
}  // namespace comet
