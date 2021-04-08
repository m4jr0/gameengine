// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "model_resource.h"

#include "comet/rendering/shader/shader_program.h"
#include "nlohmann/json.hpp"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace resource {
bool ModelResource::Import() {
  core::Logger::Get(core::LoggerType::Resource)->Debug("Import");

  auto shader_program = std::make_shared<rendering::ShaderProgram>(
      "assets/shaders/model_shader.vs", "assets/shaders/model_shader.fs");

  shader_program->Initialize();
  model_ =
      std::make_shared<game_object::Model>(file_system_path_, shader_program);

  return true;
}

bool ModelResource::Export() {
  core::Logger::Get(core::LoggerType::Resource)->Debug("Export");
  return true;
}

bool ModelResource::Dump() {
  // if (model_ == nullptr) return false;
  return true;
}

bool ModelResource::Load() { return true; }

nlohmann::json ModelResource::GetMetaData() { return nlohmann::json(); }

void ModelResource::Destroy() {}
}  // namespace resource
}  // namespace comet
