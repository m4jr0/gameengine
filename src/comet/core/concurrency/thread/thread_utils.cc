// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "thread_utils.h"

#include <atomic>
#include <thread>

namespace comet {
namespace thread {
static_assert(std::atomic<ThreadId>::is_always_lock_free,
              "std::atomic<ThreadId> needs to be always lock-free. Unsupported "
              "architecture");
static std::atomic<ThreadId> thread_id_counter{0};

ThreadId GetThreadId() {
  static thread_local ThreadId thread_id{thread_id_counter.fetch_add(1)};
  return thread_id;
}

usize GetConcurrentThreadCount() { return std::thread::hardware_concurrency(); }
}  // namespace thread
}  // namespace comet