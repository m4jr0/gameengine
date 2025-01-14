// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_ENGINE_H_
#define COMET_COMET_CORE_ENGINE_H_

#include "comet/core/concurrency/job/job.h"
#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/event/event.h"

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

  virtual void PreLoad();
  virtual void Load();
  virtual void PostLoad();

  virtual void PreUnload();
  virtual void Unload();
  virtual void PostUnload();
  void Exit();
  static Engine& Get();
  void OnEvent(const event::Event& event);

 private:
  bool is_initialized_{false};
  bool is_running_{false};
  bool is_exit_requested_{false};
};

std::unique_ptr<Engine> GenerateEngine();
}  // namespace comet

#endif  // COMET_COMET_CORE_ENGINE_H_
