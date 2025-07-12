// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_RESOURCE_MANAGER_H_
#define COMET_COMET_RESOURCE_RESOURCE_MANAGER_H_

#include "comet/core/essentials.h"
#include "comet/core/manager.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/tstring.h"
#include "comet/resource/handler/animation_resource_handler.h"
#include "comet/resource/handler/material_resource_handler.h"
#include "comet/resource/handler/model_resource_handler.h"
#include "comet/resource/handler/shader_module_resource_handler.h"
#include "comet/resource/handler/shader_resource_handler.h"
#include "comet/resource/handler/texture_resource_handler.h"
#include "comet/resource/resource_allocator.h"

namespace comet {
namespace resource {
class ResourceManager : public Manager {
 public:
  static ResourceManager& Get();

  ResourceManager() = default;
  ResourceManager(const ResourceManager&) = delete;
  ResourceManager(ResourceManager&&) = delete;
  ResourceManager& operator=(const ResourceManager&) = delete;
  ResourceManager& operator=(ResourceManager&&) = delete;
  virtual ~ResourceManager() = default;

  void Initialize() override;
  void Shutdown() override;

  const TString& GetRootResourcePath();

  MaterialResourceHandler* GetMaterials();
  StaticModelResourceHandler* GetStaticModels();
  SkeletalModelResourceHandler* GetSkeletalModels();
  SkeletonResourceHandler* GetSkeletons();
  AnimationClipResourceHandler* GetAnimationClips();
  ShaderModuleResourceHandler* GetShaderModules();
  ShaderResourceHandler* GetShaders();
  TextureResourceHandler* GetTextures();

 private:
  void InitializeResourcesDirectory();
  void InitializeHandlers();
  void DestroyHandlers();

  TString root_resource_path_{};
  // Good enough for something that will never be updated...
  memory::PlatformAllocator handler_allocator_{
      memory::kEngineMemoryTagResource};

  memory::FiberStackAllocator global_allocator_{
      2097152,  // 2 MiB. TODO(m4jr0): Configure this in settings.
      memory::kEngineMemoryTagResourceGlobal,
      memory::kEngineMemoryTagResourceGlobalExtended};

  memory::FiberStackAllocator scene_allocator_{
      2097152,  // 2 MiB. TODO(m4jr0): Configure this in settings.
      memory::kEngineMemoryTagResourceScene,
      memory::kEngineMemoryTagResourceSceneExtended};

  memory::UniquePtr<MaterialResourceHandler> materials_{nullptr};
  memory::UniquePtr<StaticModelResourceHandler> static_models_{nullptr};
  memory::UniquePtr<SkeletalModelResourceHandler> skeletal_models_{nullptr};
  memory::UniquePtr<SkeletonResourceHandler> skeletons_{nullptr};
  memory::UniquePtr<AnimationClipResourceHandler> animation_clips_{nullptr};
  memory::UniquePtr<ShaderModuleResourceHandler> shader_modules_{nullptr};
  memory::UniquePtr<ShaderResourceHandler> shaders_{nullptr};
  memory::UniquePtr<TextureResourceHandler> textures_{nullptr};

  memory::FiberFreeListAllocator ptr_allocator_{
      sizeof(void*), 4096, memory::kEngineMemoryTagResource};
  internal::ResourceAllocator byte_allocator_{};
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_RESOURCE_MANAGER_H_
