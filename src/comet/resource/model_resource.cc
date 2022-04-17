// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "model_resource.h"

#include "comet/rendering/driver/opengl/shader/shader_program.h"
#include "nlohmann/json.hpp"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace resource {
ModelResource::ModelResource(const std::string& path) : Resource(path){};

ModelResource::ModelResource(const ModelResource& other) : Resource(other) {
  model_ = std::make_shared<game_object::Model>(*other.model_);
}

ModelResource::ModelResource(ModelResource&& other) noexcept
    : Resource(std::move(other)), model_(std::move(other.model_)) {}

ModelResource& ModelResource::operator=(const ModelResource& other) {
  if (this == &other) {
    return *this;
  }

  Resource::operator=(other);
  model_ = std::make_shared<game_object::Model>(*other.model_);
  return *this;
}

ModelResource& ModelResource::operator=(ModelResource&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  Resource::operator=(std::move(other));
  model_ = std::make_shared<game_object::Model>(*other.model_);
  return *this;
}

std::shared_ptr<comet::game_object::Component> ModelResource::Clone() const {
  return std::make_shared<ModelResource>(*this);
}

void ModelResource::Destroy() {}

bool ModelResource::Import() {
  COMET_LOG_RESOURCE_DEBUG("Import");

  const auto shader_program = std::make_shared<rendering::gl::ShaderProgram>(
      "assets/shaders/model_shader.vs", "assets/shaders/model_shader.fs");

  shader_program->Initialize();
  model_ =
      std::make_shared<game_object::Model>(file_system_path_, shader_program);

  return true;
}

bool ModelResource::Export() {
  COMET_LOG_RESOURCE_DEBUG("Export");
  return true;
}

bool ModelResource::Dump() {
  // if (model_ == nullptr) return false;
  return true;
}

bool ModelResource::Load() { return true; }

std::shared_ptr<game_object::Model> ModelResource::GetModel() { return model_; }
}  // namespace resource
}  // namespace comet
