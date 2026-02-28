// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "resource_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/conf/configuration_manager.h"
#include "comet/core/file_system/file_system.h"

namespace comet {
namespace resource {
ResourceManager& ResourceManager::Get() {
  static ResourceManager singleton{};
  return singleton;
}

void ResourceManager::Initialize() {
  Manager::Initialize();
  global_allocator_.Initialize();
  scene_allocator_.Initialize();
  byte_allocator_.Initialize();

  root_resource_path_.Reserve(conf::kMaxStrValueLength);
  root_resource_path_ = COMET_CONF_TSTR(conf::kResourceRootPath);
  COMET_DISALLOW_STR_ALLOC(root_resource_path_);
  Clean(root_resource_path_);

  InitializeResourcesDirectory();
  InitializeHandlers();
}

void ResourceManager::Shutdown() {
  root_resource_path_.Destroy();
  DestroyHandlers();
  byte_allocator_.Destroy();
  scene_allocator_.Destroy();
  global_allocator_.Destroy();
  Manager::Shutdown();
}

const TString& ResourceManager::GetRootResourcePath() {
  return root_resource_path_;
}

MaterialResourceHandler* ResourceManager::GetMaterials() {
  return materials_.get();
}

StaticModelResourceHandler* ResourceManager::GetStaticModels() {
  return static_models_.get();
}

SkeletalModelResourceHandler* ResourceManager::GetSkeletalModels() {
  return skeletal_models_.get();
}

SkeletonResourceHandler* ResourceManager::GetSkeletons() {
  return skeletons_.get();
}

AnimationClipResourceHandler* ResourceManager::GetAnimationClips() {
  return animation_clips_.get();
}

ShaderModuleResourceHandler* ResourceManager::GetShaderModules() {
  return shader_modules_.get();
}

ShaderResourceHandler* ResourceManager::GetShaders() { return shaders_.get(); }

TextureResourceHandler* ResourceManager::GetTextures() {
  return textures_.get();
}

void ResourceManager::InitializeResourcesDirectory() {
  if (!Exists(root_resource_path_)) {
    CreateDirectory(root_resource_path_, true);
  }
}

void ResourceManager::InitializeHandlers() {
  ResourceHandlerDescr descr{};
  descr.root_path = root_resource_path_;
  descr.life_span_allocators.global = &global_allocator_;
  descr.life_span_allocators.scene = &scene_allocator_;
  descr.ptr_allocator = &ptr_allocator_;
  descr.byte_allocator = &byte_allocator_;

  // TODO(m4jr0): Those are wild guesses. Not sure if it should be
  // updated/configurable.
  descr.initial_capacity = 256;
  materials_ = std::make_unique<MaterialResourceHandler>(descr);

  descr.initial_capacity = 1024;
  static_models_ = std::make_unique<StaticModelResourceHandler>(descr);

  descr.initial_capacity = 128;
  skeletal_models_ = std::make_unique<SkeletalModelResourceHandler>(descr);
  skeletons_ = std::make_unique<SkeletonResourceHandler>(descr);

  descr.initial_capacity = 1024;
  animation_clips_ = std::make_unique<AnimationClipResourceHandler>(descr);

  descr.initial_capacity = 256;
  shader_modules_ = std::make_unique<ShaderModuleResourceHandler>(descr);

  descr.initial_capacity = 128;
  shaders_ = std::make_unique<ShaderResourceHandler>(descr);

  descr.initial_capacity = 2048;
  textures_ = std::make_unique<TextureResourceHandler>(descr);

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
  materials_->Destroy();
  static_models_->Destroy();
  skeletal_models_->Destroy();
  skeletons_->Destroy();
  animation_clips_->Destroy();
  shader_modules_->Destroy();
  shaders_->Destroy();
  textures_->Destroy();

  materials_ = nullptr;
  static_models_ = nullptr;
  skeletal_models_ = nullptr;
  skeletons_ = nullptr;
  animation_clips_ = nullptr;
  shader_modules_ = nullptr;
  shaders_ = nullptr;
  textures_ = nullptr;
}
}  // namespace resource
}  // namespace comet
