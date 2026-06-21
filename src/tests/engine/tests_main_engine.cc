// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "tests_pch.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "catch2/reporters/catch_reporter_event_listener.hpp"
#include "catch2/reporters/catch_reporter_registrars.hpp"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/job/job.h"
#include "comet/core/job/scheduler.h"
#include "comet/core/thread/thread.h"
#include "comet/runtime/conf/conf_manager.h"
#include "comet/runtime/conf/config_defaults.h"
#include "comet/runtime/conf/config_keys.h"
#include "comet/runtime/conf/config_value.h"
#include "comet/runtime/frame/frame_manager.h"
#include "comet/core/logger/logging.h"
#include "comet/runtime/memory/allocation_tracking.h"
#include "comet/runtime/memory/tagged_heap.h"
#include "comet/core/id/gid.h"
#include "comet/core/id/gid_pool.h"
#include "comet/core/id/string_id.h"
#include "comet/core/id/string_id_allocator.h"
#include "comet/runtime/entity/entity_manager.h"
#include "comet/runtime/event/event_manager.h"

class TestsEventListener : public Catch::EventListenerBase {
 public:
  using Catch::EventListenerBase::EventListenerBase;

  void testRunStarting(Catch::TestRunInfo const&) override {
    comet::thread::Thread::AttachMainThread();
    COMET_INITIALIZE_ALLOCATION_TRACKING();
    COMET_LOG_INITIALIZE();
    auto& configuration_manager{comet::conf::ConfManager::Get()};
    configuration_manager.Initialize();
    configuration_manager.SetBool(comet::conf::kCoreIsMainThreadWorkerDisabled,
                                  true);
    comet::job::Scheduler::Get().Initialize();

    comet::job::JobDescr job_descr{};
    job_descr.stack_size = comet::job::JobStackSize::Large;
    job_descr.priority = comet::job::JobPriority::High;
    job_descr.entry_point = [](comet::job::JobParamsHandle) {};
    comet::job::Scheduler::Get().Run(job_descr, false);

    comet::memory::TaggedHeap::Get().Initialize();
    comet::event::EventManager::Get().Initialize();
    comet::frame::FrameManager::Get().Initialize();
    comet::gid::InitializeGids();
    comet::entity::EntityManager::Get().Initialize();
  }

  void testRunEnded(Catch::TestRunStats const&) override {
    auto& scheduler{comet::job::Scheduler::Get()};
    scheduler.RequestShutdown();

    comet::entity::EntityManager::Get().Shutdown();
    comet::gid::DestroyGids();
    comet::frame::FrameManager::Get().Shutdown();
    comet::event::EventManager::Get().Shutdown();
    comet::memory::TaggedHeap::Get().Destroy();
    scheduler.Shutdown();
    comet::conf::ConfManager::Get().Shutdown();
    COMET_LOG_DESTROY();
    comet::thread::Thread::DetachMainThread();
    COMET_STRING_ID_DESTROY();
    COMET_DESTROY_ALLOCATION_TRACKING();
  }
};

CATCH_REGISTER_LISTENER(TestsEventListener)