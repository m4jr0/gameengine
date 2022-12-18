// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "resource_manager.h"

#include "comet/core/engine.h"
#include "comet/resource/material_resource.h"
#include "comet/resource/model_resource.h"
#include "comet/resource/shader_resource.h"
#include "comet/resource/texture_resource.h"

namespace comet {
namespace resource {
void ResourceCache::Set(std::unique_ptr<Resource> resource) {
  cache_[resource->id] = std::move(resource);
}

const Resource* ResourceCache::Get(ResourceId resource_id) {
  if (cache_.find(resource_id) == cache_.cend()) {
    return nullptr;
  }

  return cache_.at(resource_id).get();
}

void ResourceManager::Initialize() {
  root_resource_path_ =
      utils::filesystem::Append(utils::filesystem::GetCurrentDirectory(),
                                COMET_CONF_STR(conf::kResourceRootPath));

  InitializeResourcesDirectory();

  AddHandler<ModelHandler>(ModelResource::kResourceTypeId);
  AddHandler<MaterialHandler>(MaterialResource::kResourceTypeId);
  AddHandler<TextureHandler>(TextureResource::kResourceTypeId);
  AddHandler<ShaderHandler>(ShaderResource::kResourceTypeId);
}

void ResourceManager::Destroy() {}

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
