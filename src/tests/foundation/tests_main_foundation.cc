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

#include "comet/core/thread/thread.h"
#include "comet/core/logger/logging.h"
#include "comet/runtime/memory/allocation_tracking.h"
#include "comet/core/id/string_id.h"
#include "comet/core/id/string_id_allocator.h"

class CoreTestsEventListener : public Catch::EventListenerBase {
 public:
  using Catch::EventListenerBase::EventListenerBase;

  void testRunStarting(Catch::TestRunInfo const&) override {
    comet::thread::Thread::AttachMainThread();
    COMET_INITIALIZE_ALLOCATION_TRACKING();
    COMET_LOG_INITIALIZE();
  }

  void testRunEnded(Catch::TestRunStats const&) override {
    COMET_LOG_DESTROY();
    comet::thread::Thread::DetachMainThread();
    COMET_STRING_ID_DESTROY();
    COMET_DESTROY_ALLOCATION_TRACKING();
  }
};

CATCH_REGISTER_LISTENER(CoreTestsEventListener)