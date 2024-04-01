// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "thread_utils.h"

#include <thread>

namespace comet {
namespace thread {
static ThreadId thread_id_counter{0};

ThreadId GetThreadId() {
  static thread_local ThreadId thread_id{thread_id_counter++};
  return thread_id;
}

usize GetConcurrentThreadCount() { return std::thread::hardware_concurrency(); }
}  // namespace thread
}  // namespace comet