// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

#include "core/locator/locator.hpp"
#include "core/render/shader/shader_program.hpp"
#include "core/render/texture/texture_loader.hpp"
#include "core/game_object/game_object.hpp"
#include "core/game_object/model/model.hpp"
#include "core/game_object/physics/transform.hpp"

// Allow debugging memory leaks.
#include "debug.hpp"

// TODO(m4jr0): Remove this file (and its uses) when a proper game object
// handling will be added.
namespace koma {
std::shared_ptr<ShaderProgram> shader_program = nullptr;
std::shared_ptr<GameObject> test_game_object = nullptr;

void InitializeTmp(GLuint width, GLuint height) {
  shader_program = std::make_shared<ShaderProgram>(
    "tmp/model_shader.vs",
    "tmp/model_shader.fs"
  );

  std::string test = "tmp/model/model.obj";
  auto test_model = std::make_shared<Model>(test, shader_program);
  auto test_transform = std::make_shared<Transform>();

  test_game_object = GameObject::Create();
  test_game_object->AddComponent(test_transform);
  test_game_object->AddComponent(test_model);

  Locator::game_object_manager().AddGameObject(test_game_object);
}

void UpdateTmp() {
}

void DestroyTmp() {
  shader_program->Destroy();
}
};
