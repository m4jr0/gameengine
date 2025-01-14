// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "engine.h"

#include "comet/animation/animation_manager.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/concurrency/provider/thread_provider_manager.h"
#include "comet/core/concurrency/thread/thread.h"
#include "comet/core/conf/configuration_manager.h"
#include "comet/core/frame/frame_manager.h"
#include "comet/core/game_state_manager.h"
#include "comet/core/logic/game_logic_manager.h"
#include "comet/core/memory/allocation_tracking.h"
#include "comet/core/memory/tagged_heap.h"
#include "comet/core/type/gid.h"
#include "comet/core/type/tstring.h"
#include "comet/entity/entity_manager.h"
#include "comet/event/event.h"
#include "comet/event/event_manager.h"
#include "comet/geometry/geometry_manager.h"
#include "comet/input/input_manager.h"
#include "comet/physics/physics_manager.h"
#include "comet/rendering/camera/camera_manager.h"
#include "comet/rendering/rendering_manager.h"
#include "comet/rendering/window/window_event.h"
#include "comet/resource/resource_manager.h"
#include "comet/scene/scene_event.h"
#include "comet/scene/scene_manager.h"
#include "comet/time/time_manager.h"

#ifdef COMET_PROFILING
#include "comet/profiler/profiler_manager.h"
#endif  // COMET_PROFILIN

#ifdef COMET_DEBUG
#include "comet/rendering/debugger/debugger_displayer_manager.h"
#endif  // COMET_DEBUG

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

  auto run_callback_descr{job::GenerateJobDescr(
      job::JobPriority::High, OnSchedulerStarted, this,
      job::JobStackSize::Normal, nullptr, "scheduler_start")};

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

    auto& frame_manager{frame::FrameManager::Get()};
    auto& active_frames{frame_manager.GetInFlightFrames()};

    event::EventManager::Get().FireEvent<scene::SceneLoadRequestEvent>(
        active_frames.lead_frame);

    // To catch up time taken to render.
    f64 lag{0.0};
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

  auto& frame_manager{frame::FrameManager::Get()};
  auto& active_frames{frame_manager.GetInFlightFrames()};
  auto& frame_packet{active_frames.lead_frame};
  frame_packet->lag = lag;

  GameLogicManager::Get().Update(frame_packet);
  rendering::RenderingManager::Get().Update(active_frames.middle_frame);
  lag = frame_packet->lag;
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
  COMET_LOG_CORE_INFO("Comet is required to quit");
}

void Engine::PreLoad() {
  conf::ConfigurationManager::Get().Initialize();
  job::Scheduler::Get().Initialize();
}

void Engine::Load() {
#ifdef COMET_DEBUG
  rendering::DebuggerDisplayerManager::Get().Initialize();
#endif  // COMET_DEBUG
  rendering::RenderingManager::Get().Initialize();
  rendering::CameraManager::Get().Initialize();
  physics::PhysicsManager::Get().Initialize();

  const auto event_function{COMET_EVENT_BIND_FUNCTION(Engine::OnEvent)};

  event::EventManager::Get().Register(
      event_function, rendering::WindowCloseEvent::kStaticType_);

  animation::AnimationManager::Get().Initialize();
  entity::EntityManager::Get().Initialize();
  geometry::GeometryManager::Get().Initialize();
  GameLogicManager::Get().Initialize();
}

void Engine::PostLoad() {
  input::InputManager::Get().Initialize();
  GameStateManager::Get().Initialize();
}

void Engine::PreUnload() {
  GameStateManager::Get().Shutdown();
  scene::SceneManager::Get().Shutdown();
  time::TimeManager::Get().Shutdown();
  input::InputManager::Get().Shutdown();
  GameLogicManager::Get().Shutdown();
  geometry::GeometryManager::Get().Shutdown();
  entity::EntityManager::Get().Shutdown();
  animation::AnimationManager::Get().Shutdown();
  physics::PhysicsManager::Get().Shutdown();
  rendering::CameraManager::Get().Shutdown();
  rendering::RenderingManager::Get().Shutdown();
#ifdef COMET_DEBUG
  rendering::DebuggerDisplayerManager::Get().Shutdown();
#endif  // COMET_DEBUG
}

void Engine::Unload() {
  resource::ResourceManager::Get().Shutdown();
  event::EventManager::Get().Shutdown();
  DestroyTStrings();
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
}

void Engine::PostUnload() { Engine::engine_ = nullptr; }

void Engine::OnSchedulerStarted(job::JobParamsHandle handle) {
  auto* engine{reinterpret_cast<Engine*>(handle)};
  memory::TaggedHeap::Get().Initialize();
  thread::ThreadProviderManager::Get().Initialize();
#ifdef COMET_PROFILING
  profiler::ProfilerManager::Get().Initialize();
#endif  // COMET_PROFILING
  frame::FrameManager::Get().Initialize();
  gid::InitializeGids();
  InitializeTStrings();
  event::EventManager::Get().Initialize();
  resource::ResourceManager::Get().Initialize();
  engine->Initialize();
  engine->Run();
  job::Scheduler::Get().RequestShutdown();
}

Engine::Engine() { Engine::engine_ = this; }

void Engine::Exit() {
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

bool Engine::IsRunning() const noexcept { return is_running_; }

bool Engine::IsInitialized() const noexcept { return is_initialized_; }
}  // namespace comet
