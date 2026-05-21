// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_SCENE_SCENE_MANAGER_H_
#define COMET_RUNTIME_SCENE_SCENE_MANAGER_H_

// External. ///////////////////////////////////////////////////////////////////
#include <atomic>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/manager.h"
#include "comet/event/event.h"
#include "comet/event/event_manager.h"

namespace comet {
namespace scene {
class SceneManager : public Manager {
 public:
  static SceneManager& Get();

  SceneManager() = default;
  SceneManager(const SceneManager&) = delete;
  SceneManager(SceneManager&&) = delete;
  SceneManager& operator=(const SceneManager&) = delete;
  SceneManager& operator=(SceneManager&&) = delete;
  ~SceneManager() override = default;

  void Update();

  void LoadScene();
  usize GetExpectedEntityCount() const;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  void OnEvent(const event::Event& event);

  void RegisterEvents();
  void UnregisterEvents();

  event::EventListenerId scene_load_request_listener_id_{
      event::kInvalidEventListenerId};
};
}  // namespace scene
}  // namespace comet

#endif  // COMET_RUNTIME_SCENE_SCENE_MANAGER_H_