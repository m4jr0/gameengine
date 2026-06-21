// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_THREAD_THREAD_CONTEXT_H_
#define COMET_CORE_THREAD_THREAD_CONTEXT_H_

#include "comet/core/thread/thread.h"
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

void AttachThread(Thread* thread);
void DetachThread();
}  // namespace internal

bool IsMainThread();
bool IsThreadAttached();

ThreadId GetThreadId();
Thread* GetThread();

void Yield();

usize GetMaxConcurrentThreadCount();
usize GetConcurrentThreadCountLeft();
usize GetCurrentThreadCount();
}  // namespace thread
}  // namespace comet

#endif  // COMET_CORE_THREAD_THREAD_CONTEXT_H_