// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/thread/thread_context.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <thread>
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace thread {
static thread_local Thread* tls_current_thread{nullptr};

namespace internal {
static_assert(std::atomic<usize>::is_always_lock_free,
              "std::atomic<usize> must be always lock-free");
inline static std::atomic<usize> active_thread_count{0};

void AttachThread(Thread* thread) {
  COMET_ASSERT(tls_current_thread == nullptr, "thread_context::AttachThread",
               "thread is already attached");
  tls_current_thread = thread;
  active_thread_count.fetch_add(1, std::memory_order_acq_rel);
}

void DetachThread() {
  COMET_ASSERT(tls_current_thread != nullptr, "thread_context::DetachWorker",
               "thread is not attached");
  active_thread_count.fetch_sub(1, std::memory_order_acq_rel);
  tls_current_thread = nullptr;
}
}  // namespace internal

bool IsMainThread() { return tls_current_thread->IsMain(); }

bool IsThreadAttached() { return tls_current_thread != nullptr; }

ThreadId GetThreadId() {
  return tls_current_thread != nullptr ? tls_current_thread->GetId()
                                       : kInvalidThreadId;
}

Thread* GetThread() { return tls_current_thread; }

void Yield() { std::this_thread::yield(); }

usize GetMaxConcurrentThreadCount() {
  const auto max_thread_count{std::thread::hardware_concurrency()};
  COMET_CASSERT(max_thread_count > internal::kReservedThreadCount,
                "no thread available");
  return max_thread_count - internal::kReservedThreadCount;
}

usize GetConcurrentThreadCountLeft() {
  return GetMaxConcurrentThreadCount() -
         internal::active_thread_count.load(std::memory_order_acquire);
}

usize GetCurrentThreadCount() {
  return internal::active_thread_count.load(std::memory_order_acquire);
}
}  // namespace thread
}  // namespace comet
