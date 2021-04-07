// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "temporary_code.hpp"

#include "GL/glew.h"
#include "comet/core/engine.hpp"
#include "comet/game_object/game_object.hpp"
#include "comet/game_object/model/model.hpp"
#include "comet/game_object/physics/transform.hpp"
#include "comet/rendering/shader/shader_program.hpp"
#include "comet/rendering/texture/texture_loader.hpp"
#include "comet/resource/model_resource.hpp"
#include "comet_precompile.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#ifdef _WIN32
#include "debug_windows.hpp"
#endif  // _WIN32

// TODO(m4jr0): Remove this file (and its uses) when a proper game object
// handling will be added.
namespace comet {
std::shared_ptr<GameObject> test_game_object = nullptr;

void InitializeTmp() {
  auto model_resource =
      std::make_shared<ModelResource>("assets/models/nanosuit/model.obj");

  model_resource->Import();
  const auto test_transform = std::make_shared<Transform>();

  test_game_object = GameObject::Create();
  test_game_object->AddComponent(test_transform);
  test_game_object->AddComponent(model_resource->GetModel());

  Engine::engine()->game_object_manager()->AddGameObject(test_game_object);
}

void UpdateTmp() {}

void DestroyTmp() {}
};  // namespace comet
