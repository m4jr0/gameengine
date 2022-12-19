// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_MATERIAL_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_MATERIAL_HANDLER_H_

#include "comet_precompile.h"

#include "vulkan/vulkan.h"

#include "comet/rendering/driver/vulkan/data/vulkan_material.h"
#include "comet/rendering/driver/vulkan/data/vulkan_pipeline.h"
#include "comet/rendering/driver/vulkan/data/vulkan_shader.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_shader_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_texture_handler.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"
#include "comet/resource/material_resource.h"

namespace comet {
namespace rendering {
namespace vk {
struct MaterialLocalPacket {
  const glm::mat4* position{nullptr};
};

struct MaterialHandlerDescr : HandlerDescr {
  TextureHandler* texture_handler{nullptr};
  ShaderHandler* shader_handler{nullptr};
};

class MaterialHandler : public Handler {
 public:
  MaterialHandler() = delete;
  explicit MaterialHandler(const MaterialHandlerDescr& descr);
  MaterialHandler(const MaterialHandler&) = delete;
  MaterialHandler(MaterialHandler&& other) = delete;
  MaterialHandler& operator=(const MaterialHandler&) = delete;
  MaterialHandler& operator=(MaterialHandler&& other) = delete;
  virtual ~MaterialHandler() = default;

  void Shutdown() override;

  Material* Generate(const MaterialDescr& descr);
  Material* Generate(const resource::MaterialResource& resource);
  Material* Get(MaterialId material_id);
  Material* TryGet(MaterialId material_id);
  Material* GetOrGenerate(const MaterialDescr& descr);
  Material* GetOrGenerate(const resource::MaterialResource& resource);
  void Destroy(MaterialId material_id);
  void Destroy(Material& material);
  void UpdateInstance(Material& material, ShaderId shader_id);
  void UpdateLocal(Material& material, const MaterialLocalPacket& packet,
                   ShaderId shader_id);

 private:
  Material* GenerateInternal(const MaterialDescr& descr);
  TextureMap GenerateTextureMap(const resource::TextureMap& map);
  void Destroy(Material& material, bool is_destroying_handler);
  Sampler* GenerateSampler(SamplerId sampler_id,
                           const VkSamplerCreateInfo& info);
  Sampler* GetSampler(SamplerId sampler_id);
  Sampler* TryGetSampler(SamplerId sampler_id);
  Sampler* GetOrGenerateSampler(const resource::TextureMap& texture_map);
  void Destroy(Sampler& sampler);

  std::unordered_map<MaterialId, Material> materials_{};
  std::unordered_map<SamplerId, Sampler> samplers_{};
  TextureHandler* texture_handler_{nullptr};
  ShaderHandler* shader_handler_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_MATERIAL_HANDLER_H_
