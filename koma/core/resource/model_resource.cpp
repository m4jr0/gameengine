// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "model_resource.hpp"

#include "core/render/shader/shader_program.hpp"
#include "nlohmann/json.hpp"
#include "utils/logger.hpp"

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include "debug_windows.hpp"
#endif  // _WIN32

namespace koma {
bool ModelResource::Import() {
  Logger::Get(kLoggerKomaCoreResourceModelResource)->Debug("Import");

  auto shader_program = std::make_shared<ShaderProgram>("tmp/model_shader.vs",
                                                        "tmp/model_shader.fs");

  shader_program->Initialize();
  model_ = std::make_shared<Model>(file_system_path_, shader_program);

  return true;
}

bool ModelResource::Export() {
  Logger::Get(kLoggerKomaCoreResourceModelResource)->Debug("Export");
  return true;
}

bool ModelResource::Dump() {
  // if (model_ == nullptr) return false;
  return true;
}

bool ModelResource::Load() { return true; }

nlohmann::json ModelResource::GetMetaData() { return nlohmann::json(); }

void ModelResource::Destroy() {}
}  // namespace koma
