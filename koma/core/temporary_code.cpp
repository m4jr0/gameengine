// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

#include "core/game_object/game_object.hpp"
#include "core/game_object/model/model.hpp"
#include "core/game_object/physics/transform.hpp"
#include "core/locator/locator.hpp"
#include "core/render/shader/shader_program.hpp"
#include "core/render/texture/texture_loader.hpp"
#include "core/resource/model_resource.hpp"

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include "debug_windows.hpp"
#endif  // _WIN32

// TODO(m4jr0): Remove this file (and its uses) when a proper game object
// handling will be added.
namespace koma {
std::shared_ptr<GameObject> test_game_object = nullptr;

void InitializeTmp(GLuint width, GLuint height) {
  auto model_resource =
    std::make_shared<ModelResource>("assets/models/nanosuit/model.obj");

  model_resource->Import();
  auto test_transform = std::make_shared<Transform>();

  test_game_object = GameObject::Create();
  test_game_object->AddComponent(test_transform);
  test_game_object->AddComponent(model_resource->GetModel());

  Locator::game_object_manager().AddGameObject(test_game_object);
}

void UpdateTmp() {
}

void DestroyTmp() {
}
};
