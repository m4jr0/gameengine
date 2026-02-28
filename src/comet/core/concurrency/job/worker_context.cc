// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "worker_context.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/conf/configuration_manager.h"
#include "comet/core/conf/configuration_value.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace job {
static thread_local Worker* tls_current_worker{nullptr};

namespace internal {
void AttachWorker(Worker* worker) {
  COMET_CASSERT(tls_current_worker == nullptr,
                "Tried to attach worker, but it is already done!");
  tls_current_worker = worker;
  active_worker_count.fetch_add(1, std::memory_order_acq_rel);
}

void DetachWorker() {
  COMET_CASSERT(tls_current_worker != nullptr,
                "Tried to detach worker, but it is not attached!");
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
  COMET_ASSERT(tls_current_worker != nullptr,
               "Current worker is null! No worker has been attached for "
               "this thread!");
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

bool IsMainThreadWorkerDisabled() {
#ifdef COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
  static const bool result = [] {
    const bool is_disabled{
        COMET_CONF_BOOL(conf::kCoreIsMainThreadWorkerDisabled)};
    const auto* driver_label{COMET_CONF_STR(conf::kRenderingDriver)};
    const auto driver_type{rendering::GetDriverTypeFromStr(driver_label)};
    const bool is_rendering_multithreaded{
        rendering::IsMultithreading(driver_type)};

    return !is_rendering_multithreaded || is_disabled;
  }();
  return result;
#else
  return false;
#endif  // COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
}
}  // namespace job
}  // namespace comet
