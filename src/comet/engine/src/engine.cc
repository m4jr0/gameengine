// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_engine_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/engine/engine.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#ifdef COMET_MSVC
#include <iostream>

#include "comet/core/windows.h"
#else
#include <signal.h>
#endif  // COMET_MSVC
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/file_system/file_system.h"
#include "comet/core/id/gid.h"
#include "comet/core/id/gid_pool.h"
#include "comet/core/job/job.h"
#include "comet/core/job/job_utils.h"
#include "comet/core/job/scheduler.h"
#include "comet/core/job/scheduler_config.h"
#include "comet/core/memory/allocator/default_allocator.h"
#include "comet/engine/engine_event.h"
#include "comet/engine/frame/engine_frame_manager.h"
#include "comet/engine/profiler/profiler_data_collector.h"
#include "comet/platform/window/window_event.h"
#include "comet/render/render_manager.h"
#include "comet/runtime/animation/animation_manager.h"
#include "comet/runtime/camera/camera_manager.h"
#include "comet/runtime/conf/conf_manager.h"
#include "comet/runtime/continuation/continuation_manager.h"
#include "comet/runtime/entity/entity_manager.h"
#include "comet/runtime/environment/environment_manager.h"
#include "comet/runtime/frame/frame_manager.h"
#include "comet/runtime/frame/frame_packet.h"
#include "comet/runtime/geometry/geometry_manager.h"
#include "comet/runtime/input/input_manager.h"
#include "comet/runtime/light/light_manager.h"
#include "comet/runtime/memory/allocation_tracking.h"
#include "comet/runtime/memory/allocator/platform_allocator.h"
#include "comet/runtime/memory/allocator/stack_allocator.h"
#include "comet/runtime/memory/tagged_heap.h"
#include "comet/runtime/physics/physics_manager.h"
#include "comet/runtime/profiler/profiler.h"
#include "comet/runtime/resource/resource_manager.h"
#include "comet/runtime/scene/scene_event.h"
#include "comet/runtime/scene/scene_manager.h"
#include "comet/runtime/time/time_manager.h"

/*
>:3 Engine is fat.

engine_bootstrap
engine_scheduler_config
engine_lifecycle
engine_main_loop

*/

#ifdef COMET_PROFILING
#include "comet/runtime/profiler/profiler_manager.h"
#endif  // COMET_PROFILING

#ifdef COMET_HAS_DEBUG_UI
#include "comet/engine/debug/ui/debug_ui_manager.h"
#endif  // COMET_HAS_DEBUG_UI

namespace comet {
Engine::Engine(EngineClient* client) : client_{client} {
  COMET_ASSERT(client_ != nullptr, "Engine::Engine", "engine client is null");
  Engine::engine_ = this;
}

Engine::~Engine() {
  COMET_ASSERT(!is_initialized_, "Engine::~Engine", "engine still initialized");
}

void Engine::Populate() {
  COMET_CASSERT(!is_initialized_, "engine already initialized");
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
  COMET_ASSERT(!is_initialized_, "Engine::Initialize",
               "engine is already initialized");

  Load();
  PostLoad();
  client_->OnInitialize();
  is_initialized_ = true;
}

void Engine::Run() {
  COMET_ASSERT(is_initialized_, "Engine::Run", "engine is not initialized");
  COMET_ASSERT(!is_running_, "Engine::Run", "engine is already running");

  try {
    is_running_ = true;
    time::TimeManager::Get().Initialize();

    auto& scene_manager{scene::SceneManager::Get()};
    scene_manager.Initialize();
    environment::EnvironmentManager::Get().Initialize();

    event::EventManager::Get().FireEvent<scene::SceneLoadRequestEvent>();

    // To catch up time taken to render.
    f64 lag{.0};
    COMET_LOG_INFO(LoggerType::Engine, "Engine::Run", "engine started");

    while (is_running_) {
      if (is_exit_requested_) {
        break;
      }

      Update(lag);
    }
  } catch ([[maybe_unused]] const std::runtime_error& runtime_error) {
    COMET_LOG_ERROR(LoggerType::Engine, "Engine::Run", "runtime error", "what",
                    runtime_error.what());
    Quit();

    std::cin.get();
  } catch ([[maybe_unused]] const std::exception& exception) {
    COMET_LOG_ERROR(LoggerType::Engine, "Engine::Run", "exception", "what",
                    exception.what());
    Quit();

    std::cin.get();
  } catch (...) {
    COMET_LOG_ERROR(LoggerType::Engine, "Engine::Run",
                    "unknown failure detected", "suspected_cause",
                    "memory corruption");
    std::cin.get();
  }

  Exit();
}

void Engine::Update(f64& lag) {
  COMET_ASSERT(is_running_, "Engine::Update", "engine is not running");
  EngineFrameManager::Get().RunFrame(lag, *client_);

#ifdef COMET_PROFILING
  profiler::ProfilerDataCollector::Get().Update();
#endif  // COMET_PROFILING
}

void Engine::Stop() {
  is_running_ = false;
  COMET_LOG_INFO(LoggerType::Engine, "Engine::Stop", "engine stopped");
}

void Engine::Shutdown() {
  COMET_ASSERT(is_initialized_, "Engine::Shutdown",
               "engine is not initialized");

  client_->OnShutdown();
  PrepareShutdown();
  PreUnload();
  Unload();
  PostUnload();

  COMET_LOG_INFO(LoggerType::Engine, "Engine::Shutdown", "engine destroyed");
  COMET_LOG_DESTROY();
  COMET_STRING_ID_DESTROY();
  thread::Thread::DetachMainThread();
  COMET_DESTROY_ALLOCATION_TRACKING();

  is_initialized_ = false;
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
  COMET_ASSERT(engine != nullptr, "Engine::OnSchedulerStarted",
               "engine is null");

  memory::TaggedHeap::Get()
      .Initialize();  // >:3 Clean this stuff, like create functions and so on.

  memory::AttachDefaultAllocator(&engine->core_default_allocator_);

  InitializeFileSystem({
      .get_scratch_allocator = &frame::GetFrameAllocator(),
  });

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

void Engine::Exit() {
  event::EventManager::Get().FireEventNow<ApplicationQuitEvent>();
  Stop();
  COMET_LOG_INFO(LoggerType::Engine, "Engine::Exit", "engine quit");
}

Engine& Engine::Get() {
  COMET_ASSERT(Engine::engine_ != nullptr, "Engine::Get",
               "engine singleton is null");
  return *Engine::engine_;
}

void Engine::OnEvent(const event::Event& event) {
  const auto& event_type{event.GetType()};

  if (event_type == platform::WindowCloseEvent::kStaticType_) {
    COMET_LOG_DEBUG(LoggerType::Engine, "Engine::OnEvent",
                    "window close event");
    Quit();
  }
}

void Engine::RegisterEvents() {
  const auto event_function{
      [this](const event::Event& event) { OnEvent(event); }};

  window_close_listener_id_ = event::EventManager::Get().Register(
      event_function, platform::WindowCloseEvent::kStaticType_);
  COMET_ASSERT(window_close_listener_id_ != event::kInvalidEventListenerId,
               "Engine::RegisterEvents",
               "window close listener registration failed");
}

void Engine::UnregisterEvents() {
  if (window_close_listener_id_ != event::kInvalidEventListenerId) {
    event::EventManager::Get().Unregister(window_close_listener_id_);
    window_close_listener_id_ = event::kInvalidEventListenerId;
  }
}

#ifdef COMET_WINDOWS
static BOOL WINAPI HandleConsole(DWORD window_event) {
  switch (window_event) {
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
      Engine::Get().Quit();
      return TRUE;

    default:
      break;
  }

  return FALSE;
}
#endif  // COMET_WINDOWS

void Engine::RegisterSignalHandlers() {
#ifdef COMET_WINDOWS
  if (!SetConsoleCtrlHandler(static_cast<PHANDLER_ROUTINE>(HandleConsole),
                             TRUE)) {
    std::cerr
        << "CometEditor::OnLoadBefore: console handler registration failed"
        << '\n';
  }
#endif  // COMET_WINDOWS

#ifdef COMET_UNIX
  struct sigaction sig_handler;
  sig_handler.sa_handler = [](s32) { CometEditor::Get().Quit(); };

  sigemptyset(&sig_handler.sa_mask);
  sig_handler.sa_flags = 0;

  sigaction(SIGINT, &sig_handler, NULL);
#endif  // COMET_UNIX
}

void Engine::PreLoad() {
  InitializeScheduler();

  ContinuationManager::Get().Initialize();
}

void Engine::Load() {
  RegisterSignalHandlers();
  render::RenderManager::Get().Initialize();
  light::LightManager::Get().Initialize();
  camera::CameraManager::Get().Initialize();
  physics::PhysicsManager::Get().Initialize();

  RegisterEvents();

  animation::AnimationManager::Get().Initialize();
  entity::EntityManager::Get().Initialize();
  geometry::GeometryManager::Get().Initialize();
  EngineFrameManager::Get().Initialize();
#ifdef COMET_HAS_DEBUG_UI
  debug::DebugUiManager::Get().Initialize();
#endif  // COMET_HAS_DEBUG_UI
}

void Engine::PostLoad() {
  if (scheduler_config_.is_main_thread_worker_disabled) {
    job::CounterGuard guard{};

    job::Scheduler::Get().KickOnMainThread(job::GenerateMainThreadJobDescr(
        [](job::MainThreadParamsHandle params_handle) {
          auto* engine{reinterpret_cast<Engine*>(params_handle)};
          COMET_ASSERT(engine != nullptr, "Engine::PostLoad", "engine is null");

          engine->glfw_input_backend_.AttachWindow(
              render::RenderManager::Get().GetWindowHandle());

          input::InputManager::Get().AttachBackend(
              &engine->glfw_input_backend_);
          input::InputManager::Get().Initialize();
        },
        this, guard.GetCounter()));

    guard.Wait();
  } else {
    input::InputManager::Get().Initialize();
  }
}

void Engine::PrepareShutdown() {
#ifdef COMET_HAS_DEBUG_UI
  debug::DebugUiManager::Get().PrepareShutdown();
#endif  // COMET_HAS_DEBUG_UI
  environment::EnvironmentManager::Get().PrepareShutdown();
  scene::SceneManager::Get().PrepareShutdown();
  time::TimeManager::Get().PrepareShutdown();
  input::InputManager::Get().PrepareShutdown();
  EngineFrameManager::Get().PrepareShutdown();
  geometry::GeometryManager::Get().PrepareShutdown();
  entity::EntityManager::Get().PrepareShutdown();
  animation::AnimationManager::Get().PrepareShutdown();
  physics::PhysicsManager::Get().PrepareShutdown();
  camera::CameraManager::Get().PrepareShutdown();
  light::LightManager::Get().PrepareShutdown();
  render::RenderManager::Get().PrepareShutdown();
  resource::ResourceManager::Get().PrepareShutdown();
  event::EventManager::Get().PrepareShutdown();
  frame::FrameManager::Get().PrepareShutdown();
#ifdef COMET_PROFILING
  profiler::ProfilerManager::Get().PrepareShutdown();
#endif  // COMET_PROFILING
  conf::ConfManager::Get().PrepareShutdown();
}

void Engine::PreUnload() {
  UnregisterEvents();

#ifdef COMET_HAS_DEBUG_UI
  debug::DebugUiManager::Get().Shutdown();
#endif  // COMET_HAS_DEBUG_UI
  environment::EnvironmentManager::Get().Shutdown();
  scene::SceneManager::Get().Shutdown();
  time::TimeManager::Get().Shutdown();

  input::InputManager::Get().Shutdown();
  glfw_input_backend_.DetachWindow();

  EngineFrameManager::Get().Shutdown();
  geometry::GeometryManager::Get().Shutdown();
  entity::EntityManager::Get().Shutdown();
  animation::AnimationManager::Get().Shutdown();
  physics::PhysicsManager::Get().Shutdown();
  camera::CameraManager::Get().Shutdown();
  light::LightManager::Get().Shutdown();
  render::RenderManager::Get().Shutdown();
}

void Engine::Unload() {
  resource::ResourceManager::Get().Shutdown();
#ifdef COMET_PROFILING
  profiler::ProfilerManager::Get().Shutdown();
#endif  // COMET_PROFILING
  event::EventManager::Get().Shutdown();
  gid::DestroyGids();
  frame::FrameManager::Get().Shutdown();
  ContinuationManager::Get().Shutdown();

  ShutdownScheduler();

  memory::DetachDefaultAllocator();

  conf::ConfManager::Get().Shutdown();

  memory::TaggedHeap::Get().Destroy();

  is_running_ = false;
  is_exit_requested_ = false;
}

void Engine::PostUnload() {
  Engine::engine_ = nullptr;

  ShutdownFileSystem();
}

void Engine::InitializeScheduler() {
  const auto large_fiber_count{
      static_cast<usize>(COMET_CONF_U16(conf::kCoreLargeFiberCount))};
  const auto gigantic_fiber_count{
      static_cast<usize>(COMET_CONF_U16(conf::kCoreGiganticFiberCount))};
  const auto job_queue_capacity{
      static_cast<usize>(COMET_CONF_U16(conf::kCoreJobQueueCount))};
  const auto counter_count{
      static_cast<usize>(COMET_CONF_U16(conf::kCoreJobCounterCount))};

  usize total_fiber_count{large_fiber_count + gigantic_fiber_count};
  usize total_fiber_stack_size{
      large_fiber_count *
          fiber::Fiber::GetAllocatedStackSize(fiber::kLargeStackSize) +
      gigantic_fiber_count *
          fiber::Fiber::GetAllocatedStackSize(fiber::kGiganticStackSize)};

#ifdef COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
  const auto external_library_fiber_count{
      static_cast<usize>(COMET_CONF_U16(conf::kCoreExternalLibraryFiberCount))};

  total_fiber_count += external_library_fiber_count;
  total_fiber_stack_size += external_library_fiber_count *
                            fiber::Fiber::GetAllocatedStackSize(
                                fiber::kNormalExternalLibraryStackSize);
#endif  // COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT

  scheduler_fiber_object_allocator_ = memory::PlatformStackAllocator{
      total_fiber_count * sizeof(fiber::Fiber) + alignof(fiber::Fiber),
      kEngineMemoryTagFiber};
  scheduler_fiber_object_allocator_.Initialize();

  scheduler_fiber_stack_allocator_ = memory::PlatformStackAllocator{
      total_fiber_stack_size, kEngineMemoryTagFiber};
  scheduler_fiber_stack_allocator_.Initialize();

  scheduler_counter_allocator_ = memory::PlatformStackAllocator{
      counter_count * sizeof(job::Counter) + alignof(job::Counter),
      kEngineMemoryTagFiber};
  scheduler_counter_allocator_.Initialize();

  scheduler_config_ = {
      .worker_lifecycle = {
          .attach =
              [] {
                frame::AttachFrameAllocator(
                    frame::FrameManager::Get().GetFrameAllocator());
                frame::AttachDoubleFrameAllocator(
                    frame::FrameManager::Get().GetDoubleFrameAllocator());
              },
          .detach =
              [] {
                frame::DetachFrameAllocator();
                frame::DetachDoubleFrameAllocator();
              },
          .io_attach =
              [] {
                AttachTStringAllocator(
                    frame::FrameManager::Get().GetFrameAllocator());
              },
          .io_detach = [] { DetachTStringAllocator(); },
      },
      .job_queue_allocator = &scheduler_job_queue_allocator_,
      .worker_allocator = &scheduler_worker_allocator_,
      .fiber_queue_allocator = &scheduler_fiber_queue_allocator_,
      .fiber_object_allocator = &scheduler_fiber_object_allocator_,
      .fiber_stack_allocator = &scheduler_fiber_stack_allocator_,
      .fiber_life_cycle_allocator = &scheduler_fiber_life_cycle_allocator_,
      .counter_queue_allocator = &scheduler_counter_queue_allocator_,
      .counter_allocator = &scheduler_counter_allocator_,
      .main_thread_queue_allocator = &scheduler_main_thread_queue_allocator_,
      .job_queue_capacity = job_queue_capacity,
      .counter_count = counter_count,
      .large_fiber_count = large_fiber_count,
      .gigantic_fiber_count = gigantic_fiber_count,

#ifdef COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT  // >:3 Useless now?
      .external_library_fiber_count = external_library_fiber_count,
#endif  // COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT

      .fiber_life_cycle_queue_capacity = 128,
      .main_thread_queue_capacity = 16,
      .forced_fiber_worker_count =
          COMET_CONF_U8(conf::kCoreForcedFiberWorkerCount),
      .forced_io_worker_count = COMET_CONF_U8(conf::kCoreForcedIOWorkerCount),
      .default_io_worker_count = 2,
      .promotion_interval = 1000,
      .is_main_thread_worker_disabled = engine::IsMainThreadWorkerDisabled(),
  };

  auto& scheduler{job::Scheduler::Get()};
  scheduler.AttachConfig(scheduler_config_);
  scheduler.Initialize();
}

void Engine::ShutdownScheduler() {
  auto& scheduler{job::Scheduler::Get()};
  scheduler.Shutdown();
  scheduler.DetachConfig();

  scheduler_counter_allocator_.Destroy();
  scheduler_fiber_stack_allocator_.Destroy();
  scheduler_fiber_object_allocator_.Destroy();
}
}  // namespace comet
