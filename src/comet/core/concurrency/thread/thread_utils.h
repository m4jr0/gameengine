// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONCURRENCY_THREAD_THREAD_UTILS_H_
#define COMET_COMET_CORE_CONCURRENCY_THREAD_THREAD_UTILS_H_

#include "comet/core/essentials.h"

namespace comet {
namespace thread {
using ThreadId = usize;
constexpr auto kInvalidThreadId{static_cast<ThreadId>(-1)};

ThreadId GetThreadId();
usize GetConcurrentThreadCount();
}  // namespace thread
}  // namespace comet

#endif  // COMET_COMET_CORE_JOB_TCONCURRENCY_THREADAD_UTILS_H_