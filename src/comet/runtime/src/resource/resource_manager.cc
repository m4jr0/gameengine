// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/resource/resource_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/runtime/conf/conf_manager.h"
#include "comet/core/file_system/file_system.h"

namespace comet {
namespace resource {
ResourceManager& ResourceManager::Get() {
  static ResourceManager singleton{};
  return singleton;
}

const TString& ResourceManager::GetRootResourcePath() {
  COMET_ASSERT(!root_resource_path_.IsEmpty(),
               "ResourceManager::GetRootResourcePath",
               "root resource path is empty");
  return root_resource_path_;
}

MaterialResourceHandler* ResourceManager::GetMaterials() {
  COMET_ASSERT(materials_ != nullptr, "ResourceManager::GetMaterials",
               "material handler is null");
  return materials_.get();
}

StaticModelResourceHandler* ResourceManager::GetStaticModels() {
  COMET_ASSERT(materials_ != nullptr, "ResourceManager::GetMaterials",
               "material handler is null");
  return static_models_.get();
}

SkeletalModelResourceHandler* ResourceManager::GetSkeletalModels() {
  COMET_ASSERT(skeletal_models_ != nullptr,
               "ResourceManager::GetSkeletalModels",
               "skeletal model handler is null");
  return skeletal_models_.get();
}

SkeletonResourceHandler* ResourceManager::GetSkeletons() {
  COMET_ASSERT(skeletons_ != nullptr, "ResourceManager::GetSkeletons",
               "skeleton handler is null");
  return skeletons_.get();
}

AnimationClipResourceHandler* ResourceManager::GetAnimationClips() {
  COMET_ASSERT(animation_clips_ != nullptr,
               "ResourceManager::GetAnimationClips",
               "animation clip handler is null");
  return animation_clips_.get();
}

ShaderModuleResourceHandler* ResourceManager::GetShaderModules() {
  COMET_ASSERT(shader_modules_ != nullptr, "ResourceManager::GetShaderModules",
               "shader module handler is null");
  return shader_modules_.get();
}

ShaderResourceHandler* ResourceManager::GetShaders() {
  COMET_ASSERT(shaders_ != nullptr, "ResourceManager::GetShaders",
               "shader handler is null");
  return shaders_.get();
}

TextureResourceHandler* ResourceManager::GetTextures() {
  COMET_ASSERT(textures_ != nullptr, "ResourceManager::GetTextures",
               "texture handler is null");
  return textures_.get();
}

void ResourceManager::OnInitialize() {
  global_allocator_.Initialize();
  scene_allocator_.Initialize();
  ptr_allocator_.Initialize();
  byte_allocator_.Initialize();

  root_resource_path_.Reserve(conf::kMaxStrValueLength);
  root_resource_path_ = COMET_CONF_TSTR(conf::kResourceRootPath);
  COMET_DISALLOW_STR_ALLOC(root_resource_path_);
  Clean(root_resource_path_);
  COMET_ASSERT(!root_resource_path_.IsEmpty(), "ResourceManager::OnInitialize",
               "root resource path is empty");

  InitializeResourcesDirectory();
  InitializeHandlers();
}

void ResourceManager::OnShutdown() {
  root_resource_path_.Release();
  DestroyHandlers();
  byte_allocator_.Destroy();
  ptr_allocator_.Destroy();
  scene_allocator_.Destroy();
  global_allocator_.Destroy();
}

void ResourceManager::InitializeResourcesDirectory() {
  if (!Exists(root_resource_path_)) {
    CreateDirectory(root_resource_path_, true);
    COMET_ASSERT(Exists(root_resource_path_),
                 "ResourceManager::InitializeResourcesDirectory",
                 "failed to create resource root directory", "path",
                 root_resource_path_);
  }

  COMET_ASSERT(!root_resource_path_.IsEmpty(),
               "ResourceManager::InitializeResourcesDirectory",
               "root resource path is empty");
}

void ResourceManager::InitializeHandlers() {
  ResourceHandlerDescr descr{};
  descr.root_path = root_resource_path_;
  descr.life_span_allocators.global = &global_allocator_;
  descr.life_span_allocators.scene = &scene_allocator_;
  descr.life_span_allocators.immortal = &platform_allocator_;
  descr.ptr_allocator = &ptr_allocator_;
  descr.byte_allocator = &byte_allocator_;

  // TODO(m4jr0): Those are wild guesses. Not sure if it should be
  // updated/configurable.
  descr.initial_capacity = 1024;
  descr.memory_tag = kEngineMemoryTagResourceAnimationHandler;
  animation_clips_ = std::make_unique<AnimationClipResourceHandler>(descr);
  COMET_ASSERT(animation_clips_ != nullptr,
               "ResourceManager::InitializeHandlers",
               "animation clip handler allocation failed");

  descr.initial_capacity = 256;
  descr.memory_tag = kEngineMemoryTagResourceMaterialHandler;
  materials_ = std::make_unique<MaterialResourceHandler>(descr);
  COMET_ASSERT(materials_ != nullptr, "ResourceManager::InitializeHandlers",
               "material handler allocation failed");

  descr.initial_capacity = 1024;
  descr.memory_tag = kEngineMemoryTagResourceStaticModelHandler;
  static_models_ = std::make_unique<StaticModelResourceHandler>(descr);
  COMET_ASSERT(static_models_ != nullptr, "ResourceManager::InitializeHandlers",
               "static model handler allocation failed");

  descr.initial_capacity = 128;
  descr.memory_tag = kEngineMemoryTagResourceSkeletalModelHandler;
  skeletal_models_ = std::make_unique<SkeletalModelResourceHandler>(descr);
  COMET_ASSERT(skeletal_models_ != nullptr,
               "ResourceManager::InitializeHandlers",
               "skeletal model handler allocation failed");

  descr.initial_capacity = 128;
  descr.memory_tag = kEngineMemoryTagResourceSkeletonHandler;
  skeletons_ = std::make_unique<SkeletonResourceHandler>(descr);
  COMET_ASSERT(skeletons_ != nullptr, "ResourceManager::InitializeHandlers",
               "skeleton handler allocation failed");

  descr.initial_capacity = 256;
  descr.memory_tag = kEngineMemoryTagResourceShaderModuleHandler;
  shader_modules_ = std::make_unique<ShaderModuleResourceHandler>(descr);
  COMET_ASSERT(shader_modules_ != nullptr,
               "ResourceManager::InitializeHandlers",
               "shader module handler allocation failed");

  descr.initial_capacity = 128;
  descr.memory_tag = kEngineMemoryTagResourceShaderHandler;
  shaders_ = std::make_unique<ShaderResourceHandler>(descr);
  COMET_ASSERT(shaders_ != nullptr, "ResourceManager::InitializeHandlers",
               "shader handler allocation failed");

  descr.initial_capacity = 2048;
  descr.memory_tag = kEngineMemoryTagResourceTextureHandler;
  textures_ = std::make_unique<TextureResourceHandler>(descr);
  COMET_ASSERT(textures_ != nullptr, "ResourceManager::InitializeHandlers",
               "texture handler allocation failed");

  materials_->Initialize();
  static_models_->Initialize();
  skeletal_models_->Initialize();
  skeletons_->Initialize();
  animation_clips_->Initialize();
  shader_modules_->Initialize();
  shaders_->Initialize();
  textures_->Initialize();
}

void ResourceManager::DestroyHandlers() {
  if (materials_ != nullptr) {
    materials_->Destroy();
    materials_ = nullptr;
  }

  if (static_models_ != nullptr) {
    static_models_->Destroy();
    static_models_ = nullptr;
  }

  if (skeletal_models_ != nullptr) {
    skeletal_models_->Destroy();
    skeletal_models_ = nullptr;
  }

  if (skeletons_ != nullptr) {
    skeletons_->Destroy();
    skeletons_ = nullptr;
  }

  if (animation_clips_ != nullptr) {
    animation_clips_->Destroy();
    animation_clips_ = nullptr;
  }

  if (shader_modules_ != nullptr) {
    shader_modules_->Destroy();
    shader_modules_ = nullptr;
  }

  if (shaders_ != nullptr) {
    shaders_->Destroy();
    shaders_ = nullptr;
  }

  if (textures_ != nullptr) {
    textures_->Destroy();
    textures_ = nullptr;
  }
}
}  // namespace resource
}  // namespace comet
