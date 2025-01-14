// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_MATERIAL_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_MATERIAL_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/map.h"
#include "comet/math/math_commons.h"
#include "comet/math/matrix.h"
#include "comet/rendering/driver/vulkan/data/vulkan_material.h"
#include "comet/rendering/driver/vulkan/data/vulkan_pipeline.h"
#include "comet/rendering/driver/vulkan/data/vulkan_shader.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_texture_handler.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"
#include "comet/resource/material_resource.h"

namespace comet {
namespace rendering {
namespace vk {
struct MaterialHandlerDescr : HandlerDescr {
  TextureHandler* texture_handler{nullptr};
};

class MaterialHandler : public Handler {
 public:
  MaterialHandler() = delete;
  explicit MaterialHandler(const MaterialHandlerDescr& descr);
  MaterialHandler(const MaterialHandler&) = delete;
  MaterialHandler(MaterialHandler&&) = delete;
  MaterialHandler& operator=(const MaterialHandler&) = delete;
  MaterialHandler& operator=(MaterialHandler&&) = delete;
  virtual ~MaterialHandler() = default;

  void Initialize() override;
  void Shutdown() override;

  Material* Generate(const MaterialDescr& descr);
  Material* Generate(const resource::MaterialResource* resource);
  Material* Get(MaterialId material_id);
  Material* TryGet(MaterialId material_id);
  Material* GetOrGenerate(const MaterialDescr& descr);
  Material* GetOrGenerate(const resource::MaterialResource* resource);
  void Destroy(MaterialId material_id);
  void Destroy(Material* material);

 private:
  Material* GenerateInternal(const MaterialDescr& descr);
  TextureMap GenerateTextureMap(const resource::TextureMap* map);
  void Destroy(Material* material, bool is_destroying_handler);
  Sampler* GenerateSampler(SamplerId sampler_id,
                           const VkSamplerCreateInfo& info);
  Sampler* GetSampler(SamplerId sampler_id);
  Sampler* TryGetSampler(SamplerId sampler_id);
  Sampler* GetOrGenerateSampler(const resource::TextureMap* texture_map);
  void Destroy(Sampler* sampler);

  memory::FiberStackAllocator allocator_{
      math::Max(sizeof(Pair<MaterialId, Material>),
                sizeof(Pair<SamplerId, Sampler>)),
      256, memory::kEngineMemoryTagRendering};
  Map<MaterialId, Material*> materials_{};
  Map<SamplerId, Sampler*> samplers_{};
  TextureHandler* texture_handler_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_MATERIAL_HANDLER_H_
