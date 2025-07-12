// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONCURRENCY_THREAD_THREAD_CONTEXT_H_
#define COMET_COMET_CORE_CONCURRENCY_THREAD_THREAD_CONTEXT_H_

#include "comet/core/concurrency/thread/thread.h"
#include "comet/core/essentials.h"

namespace comet {
namespace thread {
namespace internal {
inline static constexpr auto kReservedThreadCount{
#ifdef COMET_RESERVE_SYSTEM_THREADS
    2
#else
    0
#endif  // COMET_RESERVE_SYSTEM_THREADS
};

static_assert(std::atomic<usize>::is_always_lock_free,
              "std::atomic<usize> needs to be always lock-free. Unsupported "
              "architecture");
inline static std::atomic<usize> active_thread_count{0};

void AttachThread(Thread* thread);
void DetachThread();
}  // namespace internal

bool IsThreadAttached();
ThreadId GetThreadId();
Thread* GetThread();
void Yield();
usize GetMaxConcurrentThreadCount();
usize GetConcurrentThreadCountLeft();
usize GetCurrentThreadCount();
bool IsMainThread();
}  // namespace thread
}  // namespace comet

#endif  // COMET_COMET_CORE_CONCURRENCY_THREAD_THREAD_CONTEXT_H_