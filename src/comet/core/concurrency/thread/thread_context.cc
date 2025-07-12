// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "thread_context.h"

#include <thread>

#include "comet_pch.h"

namespace comet {
namespace thread {
static thread_local Thread* tls_current_thread{nullptr};

namespace internal {
void AttachThread(Thread* thread) {
  COMET_ASSERT(tls_current_thread == nullptr,
               "Tried to attach thread, but it is already done!");
  tls_current_thread = thread;
  active_thread_count.fetch_add(1, std::memory_order_acq_rel);
}

void DetachThread() {
  COMET_ASSERT(tls_current_thread != nullptr,
               "Tried to detach thread, but it is not attached!");
  active_thread_count.fetch_sub(1, std::memory_order_acq_rel);
  tls_current_thread = nullptr;
}
}  // namespace internal

bool IsThreadAttached() { return tls_current_thread != nullptr; }

ThreadId GetThreadId() {
  return tls_current_thread != nullptr ? tls_current_thread->GetId()
                                       : kInvalidThreadId;
}

Thread* GetThread() { return tls_current_thread; }

void Yield() { std::this_thread::yield(); }

usize GetMaxConcurrentThreadCount() {
  auto max_thread_count{std::thread::hardware_concurrency()};
  COMET_CASSERT(max_thread_count > internal::kReservedThreadCount,
                "No thread available!");
  return max_thread_count - internal::kReservedThreadCount;
}

usize GetConcurrentThreadCountLeft() {
  return GetMaxConcurrentThreadCount() -
         internal::active_thread_count.load(std::memory_order_acquire);
}

usize GetCurrentThreadCount() {
  return internal::active_thread_count.load(std::memory_order_acquire);
}

bool IsMainThread() { return tls_current_thread->IsMain(); }
}  // namespace thread
}  // namespace comet
