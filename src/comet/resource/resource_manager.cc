// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "resource_manager.h"

#include "comet/core/engine.h"

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
  root_resource_path_ = utils::filesystem::Append(
      utils::filesystem::GetCurrentDirectory(),
      Engine::Get().GetConfigurationManager().Get<std::string>(
          "resource_root_path"));

  InitializeResourcesDirectory();

  AddHandler<texture::TextureHandler>(
      texture::TextureResource::kResourceTypeId);
  AddHandler<model::ModelHandler>(model::ModelResource::kResourceTypeId);
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
