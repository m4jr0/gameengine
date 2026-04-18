#include "engine.h"
// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "engine.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/animation/animation_manager.h"
#include "comet/core/concurrency/job/job.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/concurrency/provider/thread_provider_manager.h"
#include "comet/core/concurrency/thread/thread.h"
#include "comet/core/conf/configuration_manager.h"
#include "comet/core/frame/frame_manager.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/logic/game_logic_manager.h"
#include "comet/core/memory/allocation_tracking.h"
#include "comet/core/memory/tagged_heap.h"
#include "comet/core/type/gid.h"
#include "comet/engine/engine_event.h"
#include "comet/entity/entity_manager.h"
#include "comet/event/event.h"
#include "comet/event/event_manager.h"
#include "comet/geometry/geometry_manager.h"
#include "comet/input/input_manager.h"
#include "comet/physics/physics_manager.h"
#include "comet/rendering/camera/camera_manager.h"
#include "comet/rendering/light/light_manager.h"
#include "comet/rendering/rendering_manager.h"
#include "comet/rendering/window/window_event.h"
#include "comet/resource/resource_manager.h"
#include "comet/scene/environment/environment_manager.h"
#include "comet/scene/scene_event.h"
#include "comet/scene/scene_manager.h"
#include "comet/time/time_manager.h"

#ifdef COMET_PROFILING
#include "comet/profiler/profiler_manager.h"
#endif  // COMET_PROFILING

#ifdef COMET_HAS_DEBUG_UI
#include "comet/debug_ui/debug_ui_manager.h"
#endif  // COMET_HAS_DEBUG_UI

namespace comet {
Engine::~Engine() {
  COMET_ASSERT(!is_initialized_,
               "Destructor called for engine, but it is still initialized!");
}

void Engine::Populate() {
  COMET_CASSERT(!is_initialized_,
                "Tried to initialize engine, but it is already done!");
  COMET_INITIALIZE_ALLOCATION_TRACKING();
  thread::Thread::AttachMainThread();
  COMET_LOG_INITIALIZE();
  PreLoad();

  const auto run_callback_descr{job::GenerateJobDescr(
      job::JobPriority::High, OnSchedulerStarted, this,
      job::JobStackSize::Large, nullptr, "scheduler_start")};

  auto& scheduler{job::Scheduler::Get()};
  scheduler.Run(run_callback_descr);
  Shutdown();
}

void Engine::Initialize() {
  Load();
  PostLoad();
  is_initialized_ = true;
}

void Engine::Run() {
  try {
    is_running_ = true;
    time::TimeManager::Get().Initialize();

    auto& scene_manager{scene::SceneManager::Get()};
    scene_manager.Initialize();
    scene::EnvironmentManager::Get().Initialize();

    event::EventManager::Get().FireEvent<scene::SceneLoadRequestEvent>();

    // To catch up time taken to render.
    f64 lag{.0};
    COMET_LOG_CORE_INFO("Comet started");

    while (is_running_) {
      if (is_exit_requested_) {
        break;
      }

      Update(lag);
    }
  } catch ([[maybe_unused]] const std::runtime_error& runtime_error) {
    COMET_LOG_CORE_ERROR("Runtime error: ", runtime_error.what());
    Quit();

    std::cin.get();
  } catch ([[maybe_unused]] const std::exception& exception) {
    COMET_LOG_CORE_ERROR("Exception: ", exception.what());
    Quit();

    std::cin.get();
  } catch (...) {
    COMET_LOG_CORE_ERROR(
        "Unknown failure occurred. Possible memory corruption");
    std::cin.get();
  }

  Exit();
}

void Engine::Update(f64& lag) {
  time::TimeManager::Get().Update();
  lag += time::TimeManager::Get().GetDeltaTime();

  job::CounterGuard guard{};
  auto& frame_manager{frame::FrameManager::Get()};
  auto* logic_frame_packet{frame_manager.GetLogicFramePacket()};
  logic_frame_packet->lag = lag;
  logic_frame_packet->counter = guard.GetCounter();

  auto* rendering_frame_packet{frame_manager.GetRenderingFramePacket()};
  rendering_frame_packet->counter = guard.GetCounter();

  GameLogicManager::Get().Update(logic_frame_packet);
  rendering::RenderingManager::Get().Update(rendering_frame_packet);

  guard.Wait();
  logic_frame_packet->counter = nullptr;
  rendering_frame_packet->counter = nullptr;
  lag = logic_frame_packet->lag;
  frame_manager.Update();

#ifdef COMET_PROFILING
  profiler::ProfilerManager::Get().Update();
#endif  // COMET_PROFILING
}

void Engine::Stop() {
  is_running_ = false;
  COMET_LOG_CORE_INFO("Comet stopped");
}

void Engine::Shutdown() {
  COMET_ASSERT(is_initialized_,
               "Tried to shutdown engine, but it is not initialized!");
  PrepareShutdown();
  PreUnload();
  Unload();
  PostUnload();
  is_initialized_ = false;

  COMET_LOG_CORE_INFO("Comet destroyed");
  COMET_LOG_DESTROY();
  thread::Thread::DetachMainThread();
  COMET_STRING_ID_DESTROY();
  COMET_DESTROY_ALLOCATION_TRACKING();
}

void Engine::Quit() {
  if (is_exit_requested_) {
    return;
  }

  is_exit_requested_ = true;
}

bool Engine::IsRunning() const noexcept { return is_running_; }

bool Engine::IsInitialized() const noexcept { return is_initialized_; }

void Engine::OnSchedulerStarted(job::JobParamsHandle handle) {
  auto* engine{reinterpret_cast<Engine*>(handle)};
  memory::TaggedHeap::Get().Initialize();
  thread::ThreadProviderManager::Get().Initialize();
  frame::FrameManager::Get().Initialize();
  gid::InitializeGids();
  event::EventManager::Get().Initialize();
#ifdef COMET_PROFILING
  profiler::ProfilerManager::Get().Initialize();
#endif  // COMET_PROFILING
  resource::ResourceManager::Get().Initialize();
  engine->Initialize();
  engine->Run();
  job::Scheduler::Get().RequestShutdown();
}

Engine::Engine() { Engine::engine_ = this; }

void Engine::OnPreLoadBefore() {}

void Engine::OnPreLoadAfter() {}

void Engine::OnLoadBefore() {}

void Engine::OnLoadAfter() {}

void Engine::OnPostLoadBefore() {}

void Engine::OnPostLoadAfter() {}

void Engine::OnPrepareShutdownBefore() {}

void Engine::OnPrepareShutdownAfter() {}

void Engine::OnPreUnloadBefore() {}

void Engine::OnPreUnloadAfter() {}

void Engine::OnUnloadBefore() {}

void Engine::OnUnloadAfter() {}

void Engine::OnPostUnloadBefore() {}

void Engine::OnPostUnloadAfter() {}

void Engine::Exit() {
  event::EventManager::Get().FireEventNow<ApplicationQuitEvent>();
  Stop();
  COMET_LOG_CORE_INFO("Comet quit");
}

Engine& Engine::Get() { return *Engine::engine_; }

void Engine::OnEvent(const event::Event& event) {
  const auto& event_type{event.GetType()};

  if (event_type == rendering::WindowCloseEvent::kStaticType_) {
    COMET_LOG_CORE_DEBUG("Close event.");
    Quit();
  }
}

void Engine::PreLoad() {
  OnPreLoadBefore();
  conf::ConfigurationManager::Get().Initialize();
  job::Scheduler::Get().Initialize();
  OnPreLoadAfter();
}

void Engine::Load() {
  OnLoadBefore();
  rendering::RenderingManager::Get().Initialize();
  rendering::LightManager::Get().Initialize();
  rendering::CameraManager::Get().Initialize();
  physics::PhysicsManager::Get().Initialize();

  const auto event_function{
      [this](const event::Event& event) { OnEvent(event); }};

  event::EventManager::Get().Register(
      event_function, rendering::WindowCloseEvent::kStaticType_);

  animation::AnimationManager::Get().Initialize();
  entity::EntityManager::Get().Initialize();
  geometry::GeometryManager::Get().Initialize();
  GameLogicManager::Get().Initialize();
#ifdef COMET_HAS_DEBUG_UI
  debugui::DebugUiManager::Get().Initialize();
#endif  // COMET_HAS_DEBUG_UI
  OnLoadAfter();
}

void Engine::PostLoad() {
  OnPostLoadBefore();

  if (job::IsMainThreadWorkerDisabled()) {
    job::CounterGuard guard{};

    job::Scheduler::Get().KickOnMainThread(job::GenerateMainThreadJobDescr(
        [](job::MainThreadParamsHandle) {
          input::InputManager::Get().Initialize();
        },
        nullptr, guard.GetCounter()));

    guard.Wait();
  } else {
    input::InputManager::Get().Initialize();
  }

  OnPostLoadAfter();
}

void Engine::PrepareShutdown() {
  OnPrepareShutdownBefore();

#ifdef COMET_HAS_DEBUG_UI
  debugui::DebugUiManager::Get().PrepareShutdown();
#endif  // COMET_HAS_DEBUG_UI
  scene::EnvironmentManager::Get().PrepareShutdown();
  scene::SceneManager::Get().PrepareShutdown();
  time::TimeManager::Get().PrepareShutdown();
  input::InputManager::Get().PrepareShutdown();
  GameLogicManager::Get().PrepareShutdown();
  geometry::GeometryManager::Get().PrepareShutdown();
  entity::EntityManager::Get().PrepareShutdown();
  animation::AnimationManager::Get().PrepareShutdown();
  physics::PhysicsManager::Get().PrepareShutdown();
  rendering::CameraManager::Get().PrepareShutdown();
  rendering::LightManager::Get().PrepareShutdown();
  rendering::RenderingManager::Get().PrepareShutdown();
  resource::ResourceManager::Get().PrepareShutdown();
  event::EventManager::Get().PrepareShutdown();
  frame::FrameManager::Get().PrepareShutdown();
#ifdef COMET_PROFILING
  profiler::ProfilerManager::Get().PrepareShutdown();
#endif  // COMET_PROFILING
  thread::ThreadProviderManager::Get().PrepareShutdown();
  conf::ConfigurationManager::Get().PrepareShutdown();

  OnPrepareShutdownAfter();
}

void Engine::PreUnload() {
  OnPreUnloadBefore();

#ifdef COMET_HAS_DEBUG_UI
  debugui::DebugUiManager::Get().Shutdown();
#endif  // COMET_HAS_DEBUG_UI
  scene::EnvironmentManager::Get().Shutdown();
  scene::SceneManager::Get().Shutdown();
  time::TimeManager::Get().Shutdown();
  input::InputManager::Get().Shutdown();
  GameLogicManager::Get().Shutdown();
  geometry::GeometryManager::Get().Shutdown();
  entity::EntityManager::Get().Shutdown();
  animation::AnimationManager::Get().Shutdown();
  physics::PhysicsManager::Get().Shutdown();
  rendering::CameraManager::Get().Shutdown();
  rendering::LightManager::Get().Shutdown();
  rendering::RenderingManager::Get().Shutdown();

  OnPreUnloadAfter();
}

void Engine::Unload() {
  OnUnloadBefore();

  resource::ResourceManager::Get().Shutdown();
  event::EventManager::Get().Shutdown();
  gid::DestroyGids();
  frame::FrameManager::Get().Shutdown();
#ifdef COMET_PROFILING
  profiler::ProfilerManager::Get().Shutdown();
#endif  // COMET_PROFILING
  thread::ThreadProviderManager::Get().Shutdown();
  memory::TaggedHeap::Get().Destroy();
  job::Scheduler::Get().Shutdown();
  conf::ConfigurationManager::Get().Shutdown();
  is_running_ = false;
  is_exit_requested_ = false;

  OnUnloadAfter();
}

void Engine::PostUnload() {
  OnPostUnloadBefore();

  Engine::engine_ = nullptr;

  OnPostUnloadAfter();
}
}  // namespace comet
