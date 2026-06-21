// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "assetc_pch.h"
////////////////////////////////////////////////////////////////////////////////

#include "asset.h"
#include "comet/core.h"

int main(int argc, char** argv) {
  comet::memory::SystemAllocator allocator{};

  comet::COMET_INITIALIZE_ALLOCATION_TRACKING();
  comet::thread::Thread::AttachMainThread();
  COMET_LOG_INITIALIZE();

  comet::memory::AttachDefaultAllocator(&allocator);

  comet::InitializeFileSystem({
      .scratch_allocator = &allocator,
  });

  comet::job::SchedulerConfig scheduler_config{};
  scheduler_config.job_queue_allocator = &allocator;
  scheduler_config.worker_allocator = &allocator;
  scheduler_config.fiber_queue_allocator = &allocator;
  scheduler_config.fiber_object_allocator = &allocator;
  scheduler_config.fiber_stack_allocator = &allocator;
  scheduler_config.fiber_life_cycle_allocator = &allocator;
  scheduler_config.counter_queue_allocator = &allocator;
  scheduler_config.counter_allocator = &allocator;

  scheduler_config.job_queue_capacity = 1024;
  scheduler_config.counter_count = 256;
  scheduler_config.large_fiber_count = 32;
  scheduler_config.gigantic_fiber_count = 4;
  scheduler_config.fiber_life_cycle_queue_capacity = 128;
  scheduler_config.forced_fiber_worker_count = 0;
  scheduler_config.forced_io_worker_count = 2;
  scheduler_config.default_io_worker_count = 2;
  scheduler_config.promotion_interval = 1000;

  auto& scheduler{comet::job::Scheduler::Get()};
  scheduler.AttachConfig(scheduler_config);
  scheduler.Initialize();

  comet::tool::assetc::AssetcConfig config{};
  config.asset_root = COMET_CTSTRING_VIEW("assets");
  config.resource_root = COMET_CTSTRING_VIEW("resources");
  config.allocator = &allocator;
  config.force = false;

  comet::tool::assetc::AssetProcessor processor{};
  processor.Initialize(config);
  processor.Refresh();
  processor.Shutdown();

  scheduler.Shutdown();
  scheduler.DetachConfig();

  comet::ShutdownFileSystem();
  comet::memory::DetachDefaultAllocator();

  COMET_LOG_DESTROY();
  comet::thread::Thread::DetachMainThread();
  comet::COMET_DESTROY_ALLOCATION_TRACKING();

  return EXIT_SUCCESS;
}
