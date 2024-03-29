// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "catch2/reporters/catch_reporter_event_listener.hpp"
#include "catch2/reporters/catch_reporter_registrars.hpp"

#include "comet/core/conf/configuration_manager.h"
#include "comet/core/memory/memory_manager.h"
#include "comet/entity/entity_manager.h"

class TestsEventListener : public Catch::EventListenerBase {
 public:
  using Catch::EventListenerBase::EventListenerBase;

  void testRunStarting(Catch::TestRunInfo const&) override {
    auto& configuration_manager{comet::conf::ConfigurationManager::Get()};
    configuration_manager.Initialize();
    auto& memory_manager{comet::memory::MemoryManager::Get()};
    memory_manager.Initialize();
    auto& entity_manager{comet::entity::EntityManager::Get()};
    entity_manager.Initialize();
  }

  void testRunEnded(Catch::TestRunStats const&) override {
    auto& configuration_manager{comet::conf::ConfigurationManager::Get()};
    configuration_manager.Shutdown();
    auto& memory_manager{comet::memory::MemoryManager::Get()};
    memory_manager.Shutdown();
    auto& entity_manager{comet::entity::EntityManager::Get()};
    entity_manager.Shutdown();
  }
};

CATCH_REGISTER_LISTENER(TestsEventListener)