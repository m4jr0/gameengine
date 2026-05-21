// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_JOB_SCHEDULER_CONFIG_H_
#define COMET_CORE_JOB_SCHEDULER_CONFIG_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"

namespace comet {
namespace job {
struct SchedulerConfig {
  memory::Allocator* job_queue_allocator{nullptr};
  memory::Allocator* worker_allocator{nullptr};

  memory::Allocator* fiber_queue_allocator{nullptr};
  memory::Allocator* fiber_object_allocator{nullptr};
  memory::Allocator* fiber_stack_allocator{nullptr};
  memory::Allocator* fiber_life_cycle_allocator{nullptr};

  memory::Allocator* counter_queue_allocator{nullptr};
  memory::Allocator* counter_allocator{nullptr};

#ifdef COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
  memory::Allocator* main_thread_queue_allocator{nullptr};
#endif  // COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER

  usize job_queue_capacity{0};
  usize counter_count{0};

  usize large_fiber_count{0};
  usize gigantic_fiber_count{0};

#ifdef COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
  usize external_library_fiber_count{0};
#endif  // COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT

  usize fiber_life_cycle_queue_capacity{0};

#ifdef COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
  usize main_thread_queue_capacity{0};
#endif  // COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER

  u8 forced_fiber_worker_count{0};
  u8 forced_io_worker_count{0};
  u8 default_io_worker_count{2};

  u32 promotion_interval{1000};

#ifdef COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
  bool is_main_thread_worker_disabled{false};
#endif  // COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
};
}  // namespace job
}  // namespace comet

#endif  // COMET_CORE_JOB_SCHEDULER_CONFIG_H_