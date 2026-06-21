// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_ENGINE_ENGINE_H_
#define COMET_ENGINE_ENGINE_H_

#include "comet/core/essentials.h"
#include "comet/core/job/job.h"
#include "comet/core/memory/memory.h"
#include "comet/engine/engine_client.h"
#include "comet/platform/input/glfw_input_backend.h"
#include "comet/runtime/event/event.h"
#include "comet/runtime/event/event_manager.h"
#include "comet/runtime/memory/memory_tag.h"

namespace comet {
class Engine {
 public:
  explicit Engine(EngineClient* client);
  Engine(const Engine&) = delete;
  Engine(Engine&&) = delete;
  Engine& operator=(const Engine&) = delete;
  Engine& operator=(Engine&&) = delete;
  virtual ~Engine();

  void Populate();
  void Initialize();
  void Run();
  void Update(f64& lag);
  void Stop();
  void Shutdown();
  void Quit();

  bool IsRunning() const noexcept;
  bool IsInitialized() const noexcept;

 private:
  // Not accessible to prevent some spaghetti code.
  inline static Engine* engine_{nullptr};
  static void OnSchedulerStarted(job::JobParamsHandle handle);

  void Exit();

  static Engine& Get();

  void OnEvent(const event::Event& event);

  void RegisterEvents();
  void UnregisterEvents();

  void RegisterSignalHandlers();

  void PreLoad();
  void Load();
  void PostLoad();

  void PrepareShutdown();
  void PreUnload();
  void Unload();
  void PostUnload();

  void InitializeScheduler();
  void ShutdownScheduler();

  bool is_initialized_{false};
  bool is_running_{false};
  bool is_exit_requested_{false};

  EngineClient* client_{nullptr};

  event::EventListenerId window_close_listener_id_{
      event::kInvalidEventListenerId};

  job::SchedulerConfig scheduler_config_{};

  input::GlfwInputBackend glfw_input_backend_{};

  memory::PlatformAllocator core_default_allocator_{kEngineMemoryTagCore};

  memory::PlatformAllocator scheduler_job_queue_allocator_{
      kEngineMemoryTagFiber};
  memory::PlatformAllocator scheduler_worker_allocator_{kEngineMemoryTagFiber};
  memory::PlatformAllocator scheduler_fiber_queue_allocator_{
      kEngineMemoryTagFiber};
  memory::PlatformAllocator scheduler_counter_queue_allocator_{
      kEngineMemoryTagFiber};
  memory::PlatformAllocator scheduler_fiber_life_cycle_allocator_{
      kEngineMemoryTagFiber};
  memory::PlatformAllocator scheduler_main_thread_queue_allocator_{
      kEngineMemoryTagMainThread};

  memory::PlatformStackAllocator scheduler_fiber_object_allocator_{};
  memory::PlatformStackAllocator scheduler_fiber_stack_allocator_{};
  memory::PlatformStackAllocator scheduler_counter_allocator_{};
};
}  // namespace comet

#endif  // COMET_ENGINE_ENGINE_H_
