// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "resource_manager.h"

#include "comet/core/engine.h"
#include "comet/resource/material_resource.h"
#include "comet/resource/model_resource.h"
#include "comet/resource/shader_module_resource.h"
#include "comet/resource/shader_resource.h"
#include "comet/resource/texture_resource.h"

namespace comet {
namespace resource {
void ResourceManager::Initialize() {
  Manager::Initialize();
  root_resource_path_ =
      utils::filesystem::Append(utils::filesystem::GetCurrentDirectory(),
                                COMET_CONF_STR(conf::kResourceRootPath));

  InitializeResourcesDirectory();

  AddHandler<MaterialHandler>(MaterialResource::kResourceTypeId);
  AddHandler<ModelHandler>(ModelResource::kResourceTypeId);
  AddHandler<ShaderHandler>(ShaderResource::kResourceTypeId);
  AddHandler<ShaderModuleHandler>(ShaderModuleResource::kResourceTypeId);
  AddHandler<TextureHandler>(TextureResource::kResourceTypeId);
}

void ResourceManager::Shutdown() {
  root_resource_path_.clear();
  handlers_.clear();
  Manager::Shutdown();
}

void ResourceManager::InitializeResourcesDirectory() {
  if (!utils::filesystem::Exists(root_resource_path_)) {
    utils::filesystem::CreateDirectory(root_resource_path_, true);
  }
}

const std::string& ResourceManager::GetRootResourcePath() {
  return root_resource_path_;
}
}  // namespace resource
}  // namespace comet
