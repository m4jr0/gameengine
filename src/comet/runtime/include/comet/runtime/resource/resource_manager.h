// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_RESOURCE_RESOURCE_MANAGER_H_
#define COMET_RUNTIME_RESOURCE_RESOURCE_MANAGER_H_

#include "comet/core/essentials.h"
#include "comet/runtime/manager.h"
#include "comet/runtime/memory/allocator/free_list_allocator.h"
#include "comet/runtime/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/runtime/memory/memory_tag.h"
#include "comet/core/string/tstring.h"
#include "comet/runtime/resource/animation/animation_resource_handler.h"
#include "comet/runtime/resource/material/material_resource_handler.h"
#include "comet/runtime/resource/model/model_resource_handler.h"
#include "comet/runtime/resource/resource_allocator.h"
#include "comet/runtime/resource/shader/shader_module_resource_handler.h"
#include "comet/runtime/resource/shader/shader_resource_handler.h"
#include "comet/runtime/resource/texture/texture_resource_handler.h"

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
  ~ResourceManager() override = default;

  const TString& GetRootResourcePath();

  MaterialResourceHandler* GetMaterials();
  StaticModelResourceHandler* GetStaticModels();
  SkeletalModelResourceHandler* GetSkeletalModels();
  SkeletonResourceHandler* GetSkeletons();
  AnimationClipResourceHandler* GetAnimationClips();
  ShaderModuleResourceHandler* GetShaderModules();
  ShaderResourceHandler* GetShaders();
  TextureResourceHandler* GetTextures();

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  void InitializeResourcesDirectory();
  void InitializeHandlers();
  void DestroyHandlers();

  TString root_resource_path_{};
  memory::PlatformAllocator platform_allocator_{
      kEngineMemoryTagResource};

  memory::FiberStackAllocator global_allocator_{
      2097152,  // 2 MiB. TODO(m4jr0): Configure this in settings.
      kEngineMemoryTagResourceGlobal,
      kEngineMemoryTagResourceGlobalExtended};

  memory::FiberStackAllocator scene_allocator_{
      2097152,  // 2 MiB. TODO(m4jr0): Configure this in settings.
      kEngineMemoryTagResourceScene,
      kEngineMemoryTagResourceSceneExtended};

  memory::UniquePtr<MaterialResourceHandler> materials_{nullptr};
  memory::UniquePtr<StaticModelResourceHandler> static_models_{nullptr};
  memory::UniquePtr<SkeletalModelResourceHandler> skeletal_models_{nullptr};
  memory::UniquePtr<SkeletonResourceHandler> skeletons_{nullptr};
  memory::UniquePtr<AnimationClipResourceHandler> animation_clips_{nullptr};
  memory::UniquePtr<ShaderModuleResourceHandler> shader_modules_{nullptr};
  memory::UniquePtr<ShaderResourceHandler> shaders_{nullptr};
  memory::UniquePtr<TextureResourceHandler> textures_{nullptr};

  memory::FiberFreeListAllocator ptr_allocator_{
      sizeof(void*), 4096, kEngineMemoryTagResource};
  internal::ResourceAllocator byte_allocator_{};
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_RUNTIME_RESOURCE_RESOURCE_MANAGER_H_
