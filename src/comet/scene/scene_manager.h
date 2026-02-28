// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_SCENE_SCENE_MANAGER_H_
#define COMET_COMET_SCENE_SCENE_MANAGER_H_

// External. ///////////////////////////////////////////////////////////////////
#include <atomic>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/manager.h"
#include "comet/entity/entity_id.h"
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

  void LoadScene();
  usize GetExpectedEntityCount() const;

 private:
  void OnEvent(const event::Event& event);
  void LoadTmp();
  void HandleLoadedModelTmp(entity::EntityId entity_id);

  usize models_to_load_count_{0};
  std::atomic<usize> loaded_model_count_tmp_{0};
  entity::EntityId character_eve_id_tmp_{entity::kInvalidEntityId};
  entity::EntityId character_vampire_id_tmp_{entity::kInvalidEntityId};
  entity::EntityId sponza_id_tmp_{entity::kInvalidEntityId};
};
}  // namespace scene
}  // namespace comet

#endif  // COMET_COMET_SCENE_SCENE_MANAGER_H_
