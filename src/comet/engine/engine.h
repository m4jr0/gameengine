// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_ENGINE_H_
#define COMET_COMET_CORE_ENGINE_H_

#include "comet/core/concurrency/job/job.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/memory.h"
#include "comet/event/event.h"
#include "comet/event/event_manager.h"

namespace comet {
class Engine {
 public:
  Engine(const Engine&) = delete;
  Engine(Engine&&) = delete;
  Engine& operator=(const Engine&) = delete;
  Engine& operator=(Engine&&) = delete;
  virtual ~Engine();

  void Populate();
  void Initialize();
  void Run();
  virtual void Update(f64& lag);
  void Stop();
  void Shutdown();
  void Quit();

  bool IsRunning() const noexcept;
  bool IsInitialized() const noexcept;

 protected:
  // Not accessible to prevent some spaghetti code.
  inline static Engine* engine_{nullptr};
  static void OnSchedulerStarted(job::JobParamsHandle handle);

  Engine();

  virtual void OnPreLoadBefore();
  virtual void OnPreLoadAfter();

  virtual void OnLoadBefore();
  virtual void OnLoadAfter();

  virtual void OnPostLoadBefore();
  virtual void OnPostLoadAfter();

  virtual void OnPrepareShutdownBefore();
  virtual void OnPrepareShutdownAfter();

  virtual void OnPreUnloadBefore();
  virtual void OnPreUnloadAfter();

  virtual void OnUnloadBefore();
  virtual void OnUnloadAfter();

  virtual void OnPostUnloadBefore();
  virtual void OnPostUnloadAfter();

  void Exit();

  static Engine& Get();

  void OnEvent(const event::Event& event);

  void RegisterEvents();
  void UnregisterEvents();

 private:
  void PreLoad();
  void Load();
  void PostLoad();

  void PrepareShutdown();
  void PreUnload();
  void Unload();
  void PostUnload();

  bool is_initialized_{false};
  bool is_running_{false};
  bool is_exit_requested_{false};

  event::EventListenerId window_close_listener_id_{
      event::kInvalidEventListenerId};
};

memory::UniquePtr<Engine> GenerateEngine();
}  // namespace comet

#endif  // COMET_COMET_CORE_ENGINE_H_
