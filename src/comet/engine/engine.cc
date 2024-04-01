// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "engine.h"

// >:3
#include "comet/core/concurrency/job/scheduler_utils.h"
#include "comet/core/file_system/file_system.h"
#include "comet/core/logger.h"
#undef Yield
// >:3

#include "comet/animation/animation_manager.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/concurrency/job/scheduler_utils.h"
#include "comet/core/conf/configuration_manager.h"
#include "comet/core/memory/memory_manager.h"
#include "comet/entity/entity_manager.h"
#include "comet/entity/factory/entity_factory_manager.h"
#include "comet/event/event.h"
#include "comet/event/event_manager.h"
#include "comet/event/window_event.h"
#include "comet/geometry/geometry_manager.h"
#include "comet/input/input_manager.h"
#include "comet/physics/physics_manager.h"
#include "comet/rendering/camera/camera_manager.h"
#include "comet/rendering/rendering_manager.h"
#include "comet/resource/resource_manager.h"
#include "comet/time/time_manager.h"

#ifdef COMET_PROFILING
#include "comet/profiler/profiler_manager.h"
#endif  // COMET_PROFILIN

#ifdef COMET_DEBUG
#include "comet/rendering/debugger/debugger_displayer_manager.h"
#endif  // COMET_DEBUG

namespace comet {
// >:3
static u32 heavy_computations{0};
static TString path;
static auto* file{COMET_TCHAR("tmp.txt")};

void WriteTmpFile(fiber::ParamsHandle data) {
  std::ofstream file_stream;
  OpenFileToWriteTo(path, file_stream);
  schar buff[512];
  auto* retrieved_computations{reinterpret_cast<u32*>(data)};
  ConvertToStr(*retrieved_computations, buff, 511);
  WriteStrToFile(path, buff);
  CloseFile(file_stream);
}

void DoSomeComputations(fiber::ParamsHandle data) {
  heavy_computations = 42424242;
  job::Scheduler::Get().Kick(job::GenerateIOJobDescr(
      WriteTmpFile, reinterpret_cast<fiber::ParamsHandle>(&heavy_computations),
      reinterpret_cast<job::Counter*>(data)));
}

void CreateTmpFile(fiber::ParamsHandle data) {
  path = GetCurrentDirectory();
  COMET_ALLOW_STR_ALLOC(path);
  path /= file;
  CreateFile(path, true);
  job::Scheduler::Get().Kick(job::GenerateJobDescr(
      job::JobPriority::Normal, DoSomeComputations, data,
      job::JobStackSize::Normal, reinterpret_cast<job::Counter*>(data)));
}

void ComputeA(fiber::ParamsHandle data) {
  COMET_LOG_GLOBAL_INFO("Executing ComputeA -- Part 1");

  fiber::Yield();

  COMET_LOG_GLOBAL_INFO("Executing ComputeA -- Part 2");
  job::Scheduler::Get().Kick(job::GenerateIOJobDescr(
      CreateTmpFile, data, reinterpret_cast<job::Counter*>(data)));
}

void ComputeB([[maybe_unused]] fiber::ParamsHandle data) {
  COMET_LOG_GLOBAL_INFO("Executing ComputeB");
}

void TestFibers() {
  auto* counter_a{job::Scheduler::Get().AllocateCounter()};

  job::Scheduler::Get().KickAndWait(
      job::GenerateJobDescr(job::JobPriority::High, ComputeA,
                            reinterpret_cast<fiber::ParamsHandle>(counter_a),
                            job::JobStackSize::Normal, counter_a));
  job::Scheduler::Get().FreeCounter(counter_a);

  auto job_b{job::GenerateJobDescr(job::JobPriority::Normal, ComputeB)};
  job::Scheduler::Get().KickAndWait(job_b);
  job::Scheduler::Get().FreeCounter(job_b.counter);
}

// >:3
Engine::~Engine() {
  COMET_ASSERT(!is_initialized_,
               "Destructor called for engine, but it is still initialized!");
}

void Engine::Populate() {
  COMET_ASSERT(!is_initialized_,
               "Tried to initialize engine, but it is already done!");
  PreLoad();

  auto start_callback_descr{
      job::GenerateJobDescr(job::JobPriority::High, OnSchedulerStarted,
                            reinterpret_cast<fiber::ParamsHandle>(this))};
  job::Scheduler::Get().Start(start_callback_descr);

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
    // To catch up time taken to render.
    f64 lag{0.0};
    frame::FrameCount frame_count{0};
    COMET_LOG_CORE_INFO("Comet started");

    while (is_running_) {
      if (is_exit_requested_) {
        break;
      }

      Update(frame_count++, lag);
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

void Engine::Update(frame::FrameCount frame_count, f64& lag) {
  time::TimeManager::Get().Update();
  lag += time::TimeManager::Get().GetDeltaTime();

  frame_count += frame::kFramePacketCount;
  auto& frame_packet{*frame::GetResolvedFramePacket(frame_count)};
  frame_packet.lag = lag;

  physics::PhysicsManager::Get().Update(frame_packet);

  frame_packet.interpolation =
      lag / time::TimeManager::Get().GetFixedDeltaTime();

  animation::AnimationManager::Get().Update(
      *frame::GetResolvedFramePacket(frame_count - 1));

  rendering::RenderingManager::Get().Update(
      *frame::GetResolvedFramePacket(frame_count - 2));

#ifdef COMET_PROFILING
  profiler::ProfilerManager::Get().Update();
#endif  // COMET_PROFILING

  lag = frame_packet.lag;
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
  Logger::Get().Dispose();
}

void Engine::Quit() {
  if (is_exit_requested_) {
    return;
  }

  is_exit_requested_ = true;
  COMET_LOG_CORE_INFO("Comet is required to quit");
}

void Engine::PreLoad() {
#ifdef COMET_PROFILING
  profiler::ProfilerManager::Get().Initialize();
#endif  // COMET_PROFILING

  conf::ConfigurationManager::Get().Initialize();
  memory::MemoryManager::Get().Initialize();
  job::Scheduler::Get().Initialize();
  event::EventManager::Get().Initialize();
  resource::ResourceManager::Get().Initialize();
}

void Engine::Load() {
#ifdef COMET_DEBUG
  rendering::DebuggerDisplayerManager::Get().Initialize();
#endif  // COMET_DEBUG
  rendering::RenderingManager::Get().Initialize();
  rendering::CameraManager::Get().Initialize();
  physics::PhysicsManager::Get().Initialize();
  input::InputManager::Get().Initialize();

  const auto event_function{COMET_EVENT_BIND_FUNCTION(Engine::OnEvent)};

  event::EventManager::Get().Register(event_function,
                                      event::WindowCloseEvent::kStaticType_);

  animation::AnimationManager::Get().Initialize();
  entity::EntityManager::Get().Initialize();
  geometry::GeometryManager::Get().Initialize();
  entity::EntityFactoryManager::Get().Initialize();
}

void Engine::PostLoad() {}

void Engine::PreUnload() {
  entity::EntityFactoryManager::Get().Shutdown();
  geometry::GeometryManager::Get().Shutdown();
  entity::EntityManager::Get().Shutdown();
  animation::AnimationManager::Get().Shutdown();
  physics::PhysicsManager::Get().Shutdown();
  input::InputManager::Get().Shutdown();
  rendering::CameraManager::Get().Shutdown();
  rendering::RenderingManager::Get().Shutdown();
#ifdef COMET_DEBUG
  rendering::DebuggerDisplayerManager::Get().Shutdown();
#endif  // COMET_DEBUG
  time::TimeManager::Get().Shutdown();

#ifdef COMET_PROFILING
  profiler::ProfilerManager::Get().Shutdown();
#endif  // COMET_PROFILING
  COMET_STRING_ID_DESTROY();
}

void Engine::Unload() {
  resource::ResourceManager::Get().Shutdown();
  event::EventManager::Get().Shutdown();
  job::Scheduler::Get().Shutdown();
  memory::MemoryManager::Get().Shutdown();
  conf::ConfigurationManager::Get().Shutdown();
  is_running_ = false;
  is_exit_requested_ = false;
}

void Engine::PostUnload() { Engine::engine_ = nullptr; }

void Engine::OnSchedulerStarted(fiber::ParamsHandle handle) {
  TestFibers();  // >:3
  auto* engine{reinterpret_cast<Engine*>(handle)};
  engine->Initialize();
  engine->Run();
}

Engine::Engine() { Engine::engine_ = this; }

void Engine::Exit() {
  Stop();
  COMET_LOG_CORE_INFO("Comet quit");
}

Engine& Engine::Get() { return *Engine::engine_; }

void Engine::OnEvent(const event::Event& event) {
  const auto& event_type{event.GetType()};

  if (event_type == event::WindowCloseEvent::kStaticType_) {
    COMET_LOG_CORE_DEBUG("Close event.");
    Quit();
  }
}

bool Engine::IsRunning() const noexcept { return is_running_; }

bool Engine::IsInitialized() const noexcept { return is_initialized_; }
}  // namespace comet
