// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "resource_manager.h"

#include "comet/core/conf/configuration_manager.h"
#include "comet/core/file_system/file_system.h"
#include "comet/resource/material_resource.h"
#include "comet/resource/model_resource.h"
#include "comet/resource/shader_module_resource.h"
#include "comet/resource/shader_resource.h"
#include "comet/resource/texture_resource.h"

namespace comet {
namespace resource {
ResourceManager& ResourceManager::Get() {
  static ResourceManager singleton{};
  return singleton;
}

void ResourceManager::Initialize() {
  Manager::Initialize();
  handler_allocator_.Initialize();
  resource_file_allocator_.Initialize();
  loading_resources_allocator_.Initialize();
  loading_resource_allocator_.Initialize();

  handlers_ = ResourceHandlers{&handler_allocator_, 6};
  root_resource_path_.Reserve(conf::kMaxStrValueLength);
  root_resource_path_ = COMET_CONF_TSTR(conf::kResourceRootPath);
  COMET_DISALLOW_STR_ALLOC(root_resource_path_);
  Clean(root_resource_path_);
  InitializeResourcesDirectory();

  AddHandler<MaterialHandler>(MaterialResource::kResourceTypeId);
  AddHandler<StaticModelHandler>(StaticModelResource::kResourceTypeId);
  AddHandler<SkeletalModelHandler>(SkeletalModelResource::kResourceTypeId);
  AddHandler<ShaderHandler>(ShaderResource::kResourceTypeId);
  AddHandler<ShaderModuleHandler>(ShaderModuleResource::kResourceTypeId);
  AddHandler<TextureHandler>(TextureResource::kResourceTypeId);
}

void ResourceManager::Shutdown() {
  root_resource_path_.Clear();

  for (auto& handler : handlers_) {
    handler.value->Shutdown();
    handler_allocator_.Deallocate(handler.value);
  }

  handlers_ = {};
  loading_resource_allocator_.Destroy();
  loading_resources_allocator_.Destroy();
  resource_file_allocator_.Destroy();
  handler_allocator_.Destroy();
  Manager::Shutdown();
}

void ResourceManager::InitializeResourcesDirectory() {
  if (!Exists(root_resource_path_)) {
    CreateDirectory(root_resource_path_, true);
  }
}

const TString& ResourceManager::GetRootResourcePath() {
  return root_resource_path_;
}

void ResourceManager::RemoveHandler(ResourceTypeId resource_type_id) {
  if (!handlers_.IsContained(resource_type_id)) {
    COMET_LOG_RESOURCE_ERROR(
        "Removing unknown handler supposedly bound to resource type ID: ",
        resource_type_id, ". Aborting.");
    return;
  }

  auto* handler{handlers_.Pop(resource_type_id)};
  handler->~ResourceHandler();
  handler_allocator_.Deallocate(handler);
}
}  // namespace resource
}  // namespace comet
