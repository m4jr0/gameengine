// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_MATERIAL_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_MATERIAL_H_

#include "comet_precompile.h"

#include "vulkan/vulkan.h"

#include "comet/rendering/driver/vulkan/vulkan_image.h"
#include "comet/rendering/driver/vulkan/vulkan_initializers.h"
#include "comet/rendering/driver/vulkan/vulkan_pipeline.h"
#include "comet/rendering/driver/vulkan/vulkan_shader.h"
#include "comet/resource/material_resource.h"

namespace comet {
namespace rendering {
namespace vk {
constexpr char kBuiltInVulkanEffectTexturedOpaque[]{
    "vulkan_effect_textured_opaque"};

using VulkanEffectId = stringid::StringId;
constexpr auto kInvalidVulkanEffectId{static_cast<VulkanEffectId>(-1)};
using VulkanMaterialId = stringid::StringId;
constexpr auto kInvalidVulkanMaterialId{static_cast<VulkanMaterialId>(-1)};

enum class VulkanMeshPassType {
  Unknown = 0,
  Forward,
  Transparency,
  DirectionalShadow
};

constexpr uindex kVulkanMeshPassTypeCount{3};

template <typename DataType>
struct VulkanShaderPassData {
 public:
  DataType& operator[](VulkanMeshPassType pass_type) {
    switch (pass_type) {
      case VulkanMeshPassType::Forward:
        COMET_ASSERT(data.size() > 0, "Shader pass data size is wrong(",
                     data.size(), "), what happened?");
        return data[0];
      case VulkanMeshPassType::Transparency:
        COMET_ASSERT(data.size() > 1, "Shader pass data size is wrong(",
                     data.size(), "), what happened?");
        return data[1];
      case VulkanMeshPassType::DirectionalShadow:
        COMET_ASSERT(data.size() > 2, "Shader pass data size is wrong(",
                     data.size(), "), what happened?");
        return data[2];
    }

    COMET_ASSERT(
        false, "Invalid mesh pass type provided: ",
        static_cast<std::underlying_type_t<VulkanMeshPassType>>(pass_type),
        "!");
    return data[0];
  }

  void Clear(DataType&& clear_value) {
    for (uindex i{0}; i < kVulkanMeshPassTypeCount; ++i) {
      data[i] = clear_value;
    }
  }

 private:
  std::array<DataType, kVulkanMeshPassTypeCount> data{nullptr};
};

struct VulkanEffect {
  VulkanShaderData* data{nullptr};
  VulkanShaderPassData<const VulkanShaderPass*> pass_data;
};

struct VulkanTextureTuple {
  const VulkanTexture* texture{nullptr};
  rendering::TextureType type{rendering::TextureType::Unknown};
};

struct VulkanMaterialDescr {
  VulkanMaterialId material_id{kInvalidVulkanMaterialId};
  VulkanShaderData* data;
  std::vector<VulkanTextureTuple> texture_tuples;

  bool operator==(const VulkanMaterialDescr& other) const;
};

struct VulkanMaterial {
  VulkanMaterialId id{kInvalidVulkanMaterialId};
  VulkanEffect effect{};
  VulkanShaderData* data{nullptr};
  VulkanShaderPassData<VkDescriptorSet> descriptor_sets;
  std::vector<VulkanTextureTuple> texture_tuples;
};

VulkanMaterialId GenerateVulkanMaterialId(const char* material_name);
VulkanMaterialId GenerateVulkanMaterialId(const std::string& material_name);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

namespace std {
template <>
struct hash<comet::rendering::vk::VulkanMaterialDescr> {
  std::size_t operator()(
      const comet::rendering::vk::VulkanMaterialDescr& k) const;
};
}  // namespace std

namespace comet {
namespace rendering {
namespace vk {
class VulkanMaterialHandler {
 public:
  VulkanMaterialHandler() = default;
  VulkanMaterialHandler(const VulkanMaterialHandler&) = delete;
  VulkanMaterialHandler(VulkanMaterialHandler&&) = delete;
  VulkanMaterialHandler& operator=(const VulkanMaterialHandler&) = delete;
  VulkanMaterialHandler& operator=(VulkanMaterialHandler&&) = delete;
  ~VulkanMaterialHandler() = default;

  void Initialize();
  void Destroy();

  template <typename... VulkanShaderModuleDescrs>
  VulkanShaderPass* GenerateShaderPass(
      VulkanPipelineDescr& pipeline_descr,
      const VulkanShaderModuleDescrs&... descrs) {
    COMET_ASSERT(pipeline_descr.pipeline_type == VulkanPipelineType::Graphics,
                 "Pipeline type provided for shader generation is wrong: ",
                 static_cast<std::underlying_type_t<VulkanPipelineType>>(
                     pipeline_descr.pipeline_type),
                 "!");

    auto* shader_pass{shader_passes_.emplace_back(new VulkanShaderPass{})};
    (shader_pass->AddStage(shader_handler_.Get(descrs.shader_path),
                           descrs.stage_bits),
     ...);

    shader_pass->ReflectLayout(device_->GetDevice());

    for (const auto& shader_stage : shader_pass->stages) {
      pipeline_descr.shader_stages.emplace_back(
          init::GetPipelineShaderStageCreateInfo(
              shader_stage.stage_bits, shader_stage.shader_module.handle));
    }

    pipeline_descr.layout = shader_pass->pipeline_layout;
    shader_pass->pipeline = GenerateGraphicsPipeline(pipeline_descr);
    return shader_pass;
  }

  VulkanMaterial* Generate(const VulkanMaterialDescr& descr);
  VulkanMaterial* Get(VulkanMaterialId material_id);
  VulkanEffectId ResolveEffect(const VulkanMaterial& material) const;

  void SetDevice(const VulkanDevice& device) noexcept;
  void SetRenderPass(VkRenderPass render_pass) noexcept;

  template <typename VulkanDescriptorBuilderDescr>
  void SetDescriptorBuilderDescr(VulkanDescriptorBuilderDescr&& descr) {
    descriptor_builder_descr_ =
        std::forward<VulkanDescriptorBuilderDescr>(descr);
  }

  const VulkanDevice& GetDevice() const noexcept;
  VkRenderPass GetRenderPass() const noexcept;

 private:
  void GenerateEffects(VulkanShaderPass* default_pass);

  const VulkanDevice* device_{nullptr};
  VkRenderPass render_pass_{VK_NULL_HANDLE};
  VulkanShaderHandler shader_handler_{};
  VulkanDescriptorBuilderDescr descriptor_builder_descr_{};
  std::vector<VulkanShaderPass*> shader_passes_{};
  std::unordered_map<VulkanEffectId, VulkanEffect> effects_{};
  std::unordered_map<VulkanMaterialId, VulkanMaterial*> cache_by_id_{};
  std::unordered_map<VulkanMaterialDescr, VulkanMaterial*> cache_by_descr_{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_MATERIAL_H_
