// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "temporary_code.h"

#include "GL/glew.h"
#include "comet/core/engine.h"
#include "comet/game_object/game_object.h"
#include "comet/game_object/model/model.h"
#include "comet/game_object/physics/transform.h"
#include "comet/rendering/shader/shader_program.h"
#include "comet/rendering/texture/texture_loader.h"
#include "comet/resource/model_resource.h"
#include "comet_precompile.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

// TODO(m4jr0): Remove this file (and its uses) when a proper game object
// handling will be added.
namespace comet {
namespace rendering {
std::shared_ptr<game_object::GameObject> test_game_object = nullptr;

void InitializeTmp() {
  auto model_resource = std::make_shared<resource::ModelResource>(
      "assets/models/nanosuit/model.obj");

  model_resource->Import();
  const auto test_transform = std::make_shared<game_object::Transform>();

  test_game_object = game_object::GameObject::Create();
  test_game_object->AddComponent(test_transform);
  test_game_object->AddComponent(model_resource->GetModel());

  core::Engine::GetEngine().GetGameObjectManager().AddGameObject(
      test_game_object);
}

void UpdateTmp() {}

void DestroyTmp() {}
}  // namespace rendering
};  // namespace comet
