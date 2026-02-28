// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "catch2/reporters/catch_reporter_event_listener.hpp"
#include "catch2/reporters/catch_reporter_registrars.hpp"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/job/job.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/concurrency/provider/thread_provider_manager.h"
#include "comet/core/concurrency/thread/thread.h"
#include "comet/core/conf/configuration_manager.h"
#include "comet/core/conf/configuration_value.h"
#include "comet/core/essentials.h"
#include "comet/core/frame/frame_manager.h"
#include "comet/core/logger.h"
#include "comet/core/memory/allocation_tracking.h"
#include "comet/core/memory/tagged_heap.h"
#include "comet/core/type/gid.h"
#include "comet/entity/entity_manager.h"
#include "comet/event/event_manager.h"

class TestsEventListener : public Catch::EventListenerBase {
 public:
  using Catch::EventListenerBase::EventListenerBase;

  void testRunStarting(Catch::TestRunInfo const&) override {
    comet::thread::Thread::AttachMainThread();
    COMET_INITIALIZE_ALLOCATION_TRACKING();
    COMET_LOG_INITIALIZE();
    auto& configuration_manager{comet::conf::ConfigurationManager::Get()};
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
    comet::thread::ThreadProviderManager::Get().Initialize();
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
    comet::thread::ThreadProviderManager::Get().Shutdown();
    comet::memory::TaggedHeap::Get().Destroy();
    scheduler.Shutdown();
    comet::conf::ConfigurationManager::Get().Shutdown();
    COMET_LOG_DESTROY();
    comet::thread::Thread::DetachMainThread();
    COMET_STRING_ID_DESTROY();
    COMET_DESTROY_ALLOCATION_TRACKING();
  }
};

CATCH_REGISTER_LISTENER(TestsEventListener)