// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "model_resource.hpp"

#include <nlohmann/json.hpp>

#include <core/render/shader/shader_program.hpp>
#include <utils/logger.hpp>

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include <debug_windows.hpp>
#endif  // _WIN32

namespace koma {
bool ModelResource::Import() {
  Logger::Get(LOGGER_KOMA_CORE_RESOURCE_MODEL_RESOURCE)->Debug("Import");

  auto shader_program = std::make_shared<ShaderProgram>(
    "tmp/model_shader.vs",
    "tmp/model_shader.fs"
  );

  shader_program->Initialize();

  this->model_ = std::make_shared<Model>(
    this->file_system_path_, shader_program
  );

  return true;
}

bool ModelResource::Export() {
  Logger::Get(LOGGER_KOMA_CORE_RESOURCE_MODEL_RESOURCE)->Debug("Export");

  return true;
}

bool ModelResource::Dump() {
  //if (!this->model_) return false;

  return true;
}

bool ModelResource::Load() {
  return true;
}

nlohmann::json ModelResource::GetMetaData() {
  return nlohmann::json();
}

void ModelResource::Destroy() {

}
}  // namespace koma
