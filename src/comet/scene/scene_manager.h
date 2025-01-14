// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_SCENE_SCENE_MANAGER_H_
#define COMET_COMET_SCENE_SCENE_MANAGER_H_

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/manager.h"
#include "comet/event/event.h"

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
  ~SceneManager() = default;

  void Initialize() override;

  void LoadScene(frame::FramePacket* packet);
  usize GetExpectedEntityCount() const;

 private:
  void OnEvent(const event::Event& event);
  void LoadTmp(frame::FramePacket* packet);
};
}  // namespace scene
}  // namespace comet

#endif  // COMET_COMET_SCENE_SCENE_MANAGER_H_
