// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_material.h"

#include "comet/rendering/driver/vulkan/vulkan_initializers.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace vk {
VulkanMaterialId GenerateVulkanMaterialId(const char* material_name) {
  return comet::utils::hash::HashCrC32(material_name);
}

VulkanMaterialId GenerateVulkanMaterialId(const std::string& material_name) {
  return GenerateVulkanMaterialId(material_name.c_str());
}

bool VulkanMaterialDescr::operator==(const VulkanMaterialDescr& other) const {
  if (texture_tuples.size() != other.texture_tuples.size()) {
    return false;
  }

  return texture_tuples.size() == 0 ||
         std::memcmp(texture_tuples.data(), other.texture_tuples.data(),
                     texture_tuples.size() * sizeof(texture_tuples[0])) == 0;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet

namespace std {
size_t hash<comet::rendering::vk::VulkanMaterialDescr>::operator()(
    const comet::rendering::vk::VulkanMaterialDescr& descr) const {
  size_t result{0};

  for (const auto& tuple : descr.texture_tuples) {
    const auto tuple_hash{comet::utils::hash::HashCombine(
        hash<VkFormat>()(tuple.texture->format),
        comet::utils::hash::HashCombine(
            hash<VkSampler>()(tuple.texture->sampler),
            comet::utils::hash::HashCombine(
                hash<uint32_t>()(tuple.texture->width),
                comet::utils::hash::HashCombine(
                    hash<uint32_t>()(tuple.texture->height),
                    hash<comet::rendering::TextureType>()(tuple.type)))))};

    result = comet::utils::hash::HashCombine(result, tuple_hash);
  }

  return result;
}
}  // namespace std

namespace comet {
namespace rendering {
namespace vk {
void VulkanMaterialHandler::Initialize() {
  shader_handler_.SetDevice(device_->GetDevice());

  VulkanPipelineDescr descr{};
  descr.pipeline_type = VulkanPipelineType::Graphics;
  descr.device = device_->GetDevice();
  descr.render_pass = render_pass_;
  descr.vertex_input_description = VulkanVertex::GetVertexDescr();
  descr.input_assembly_state = init::GetPipelineInputAssemblyStateCreateInfo(
      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  descr.rasterization_state =
      init::GetPipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);
  descr.color_blend_attachment_state =
      init::GetPipelineColorBlendAttachmentState();
  descr.multisample_state = init::GetPipelineMultisampleStateCreateInfo();
  descr.multisample_state.rasterizationSamples = device_->GetMsaaSamples();
  descr.multisample_state.sampleShadingEnable = VK_TRUE;
  descr.multisample_state.minSampleShading = .2f;
  descr.depth_stencil_state = init::GetPipelineDepthStencilStateCreateInfo(
      true, true, VK_COMPARE_OP_LESS  // New fragments should be less (lower
                                      // depth = closer).
  );

  auto* default_pass{GenerateShaderPass(
      descr,
      VulkanShaderModuleDescr{VK_SHADER_STAGE_VERTEX_BIT,
                              "shaders/vulkan/default.vk.vert"},
      VulkanShaderModuleDescr{VK_SHADER_STAGE_FRAGMENT_BIT,
                              "shaders/vulkan/default.vk.frag"})};
  GenerateEffects(default_pass);
}

void VulkanMaterialHandler::Destroy() {
  for (auto& it : cache_by_id_) {
    auto* material{it.second};

    if (material->data != nullptr) {
      delete material->data;
    }

    delete material;
  }

  cache_by_id_.clear();

  for (auto& it : effects_) {
    auto& effect{it.second};

    if (effect.data != nullptr) {
      delete effect.data;
    }

    VulkanMeshPassType mesh_pass_types[] = {
        VulkanMeshPassType::Forward, VulkanMeshPassType::Transparency,
        VulkanMeshPassType::DirectionalShadow};

    for (auto type : mesh_pass_types) {
      auto* data{effect.pass_data[type]};

      if (data == nullptr) {
        continue;
      }

      auto device{device_->GetDevice()};

      if (data->pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device, data->pipeline, VK_NULL_HANDLE);
      }

      if (data->pipeline_layout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device, data->pipeline_layout, VK_NULL_HANDLE);
      }

      for (auto& stage : data->stages) {
        if (stage.shader_module.handle == VK_NULL_HANDLE) {
          continue;
        }

        vkDestroyShaderModule(device, stage.shader_module.handle,
                              VK_NULL_HANDLE);
      }

      for (auto layout : data->descriptor_set_layouts) {
        if (layout == VK_NULL_HANDLE) {
          continue;
        }

        vkDestroyDescriptorSetLayout(device, layout, VK_NULL_HANDLE);
      }
    }
  }

  effects_.clear();

  for (auto* shader_pass : shader_passes_) {
    delete shader_pass;
  }

  shader_passes_.clear();
}

VulkanMaterial* VulkanMaterialHandler::Generate(
    const VulkanMaterialDescr& descr) {
  auto it{cache_by_descr_.find(descr)};

  if (it != cache_by_descr_.end()) {
    cache_by_id_[descr.material_id] = it->second;
    return it->second;
  }

  // TODO(m4jr0): Check allocation.
  auto* material{new VulkanMaterial{}};

  material->id = descr.material_id;
  material->data = descr.data;
  material->texture_tuples = descr.texture_tuples;
  material->descriptor_sets[VulkanMeshPassType::DirectionalShadow] =
      VK_NULL_HANDLE;

  auto builder{VulkanDescriptorBuilder::Generate(descriptor_builder_descr_)};

  for (u32 i{0}; i < material->texture_tuples.size(); ++i) {
    if (material->texture_tuples[i].type != rendering::TextureType::Diffuse) {
      continue;
    }

    const auto* texture{material->texture_tuples[i].texture};
    VkDescriptorImageInfo info{texture->sampler, texture->view,
                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    builder.Bind(i, info, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                 VK_SHADER_STAGE_FRAGMENT_BIT);
  }

  builder.Build(material->descriptor_sets[VulkanMeshPassType::Forward]);
  builder.Build(material->descriptor_sets[VulkanMeshPassType::Transparency]);
  material->effect = effects_[ResolveEffect(*material)];
  cache_by_descr_[descr] = material;
  cache_by_id_[material->id] = material;
  return material;
}

VulkanMaterial* VulkanMaterialHandler::Get(VulkanMaterialId material_id) {
  auto it{cache_by_id_.find(material_id)};

  if (it == cache_by_id_.end()) {
    COMET_ASSERT(false, "Material retrieved doesn't exist: ", material_id, "!");
    return nullptr;
  }

  return it->second;
}

void VulkanMaterialHandler::SetDevice(const VulkanDevice& device) noexcept {
  device_ = &device;
}

void VulkanMaterialHandler::SetRenderPass(VkRenderPass render_pass) noexcept {
  render_pass_ = render_pass;
}

const VulkanDevice& VulkanMaterialHandler::GetDevice() const noexcept {
  return *device_;
}

VkRenderPass VulkanMaterialHandler::GetRenderPass() const noexcept {
  return render_pass_;
}
void VulkanMaterialHandler::GenerateEffects(VulkanShaderPass* default_pass) {
  VulkanEffect effect{};
  effect.pass_data[VulkanMeshPassType::DirectionalShadow] = nullptr;
  effect.pass_data[VulkanMeshPassType::Transparency] = nullptr;
  effect.pass_data[VulkanMeshPassType::Forward] = default_pass;
  effects_.emplace(COMET_STRING_ID(kBuiltInVulkanEffectTexturedOpaque),
                   std::move(effect));
}

VulkanEffectId VulkanMaterialHandler::ResolveEffect(
    const VulkanMaterial& material) const {
  return COMET_STRING_ID(kBuiltInVulkanEffectTexturedOpaque);
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet