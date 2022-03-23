// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_ENGINE_H_
#define COMET_COMET_CORE_ENGINE_H_

#include "comet/event/event_manager.h"
#include "comet/game_object/camera/camera.h"
#include "comet/game_object/game_object_manager.h"
#include "comet/input/input_manager.h"
#include "comet/physics/physics_manager.h"
#include "comet/rendering/rendering_manager.h"
#include "comet/resource/resource_manager.h"
#include "comet/time/time_manager.h"
#include "comet_precompile.h"

namespace comet {
namespace core {
class Engine {
 public:
  static constexpr double kMsPerUpdate_ = 16.66;  // 60 Hz refresh.

  Engine(const Engine&) = delete;
  Engine(Engine&&) = delete;
  Engine& operator=(const Engine&) = delete;
  Engine& operator=(Engine&&) = delete;
  virtual ~Engine() = default;

  virtual void Initialize();
  virtual void Run();
  virtual void Stop();
  virtual void Destroy();
  virtual void Quit();

  static Engine& GetEngine();
  resource::ResourceManager& GetResourceManager();
  rendering::RenderingManager& GetRenderingManager();
  input::InputManager& GetInputManager();
  time::TimeManager& GetTimeManager();
  game_object::GameObjectManager& GetGameObjectManager();
  event::EventManager& GetEventManager();
  game_object::Camera& GetMainCamera();

  const bool is_running() const noexcept;

 protected:
  inline static Engine* engine_;

  Engine();

  virtual void Exit();
  void OnEvent(const event::Event&);

  std::unique_ptr<resource::ResourceManager> resource_manager_ = nullptr;
  std::unique_ptr<input::InputManager> input_manager_ = nullptr;
  std::unique_ptr<physics::PhysicsManager> physics_manager_ = nullptr;
  std::unique_ptr<rendering::RenderingManager> rendering_manager_ = nullptr;
  std::unique_ptr<game_object::GameObjectManager> game_object_manager_ =
      nullptr;
  std::unique_ptr<time::TimeManager> time_manager_ = nullptr;
  std::unique_ptr<event::EventManager> event_manager_ = nullptr;
  std::shared_ptr<game_object::Camera> main_camera_ = nullptr;

 private:
  bool is_running_ = false;
  bool is_exit_requested_ = false;
};

std::unique_ptr<Engine> CreateEngine();
}  // namespace core
}  // namespace comet

#endif  // COMET_COMET_CORE_ENGINE_H_
