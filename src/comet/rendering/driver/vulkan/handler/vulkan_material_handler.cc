// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_material_handler.h"

#include "comet/rendering/driver/vulkan/data/vulkan_material.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_initializer_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_material_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/material_resource.h"
#include "comet/resource/resource.h"
#include "comet/resource/texture_resource.h"

namespace comet {
namespace rendering {
namespace vk {
MaterialHandler::MaterialHandler(const MaterialHandlerDescr& descr)
    : Handler{descr},
      texture_handler_{descr.texture_handler},
      shader_handler_{descr.shader_handler},
      resource_manager_{descr.resource_manager} {
  COMET_ASSERT(texture_handler_ != nullptr, "Texture handler is null!");
  COMET_ASSERT(shader_handler_ != nullptr, "Shader handler is null!");
  COMET_ASSERT(resource_manager_ != nullptr, "Resource manager is null!");
}

void MaterialHandler::Shutdown() {
  materials_.clear();
  auto& device{context_->GetDevice()};

  for (auto& it : samplers_) {
    auto& sampler{it.second};

    if (sampler.handle != VK_NULL_HANDLE) {
      vkDestroySampler(device, sampler.handle, VK_NULL_HANDLE);
    }
  }

  samplers_.clear();

  if (shader_handler_->IsInitialized()) {
    for (auto& it : materials_) {
      Destroy(it.second, true);
    }
  }

  materials_.clear();
  Handler::Shutdown();
}

Material* MaterialHandler::Generate(const MaterialDescr& descr) {
  return GenerateInternal(descr);
}

Material* MaterialHandler::Generate(
    const resource::MaterialResource& resource) {
  MaterialDescr descr{};
  descr.id = resource.id;
  descr.diffuse_map = GenerateTextureMap(resource.descr.diffuse_map);
  descr.specular_map = GenerateTextureMap(resource.descr.specular_map);
  descr.normal_map = GenerateTextureMap(resource.descr.normal_map);

  std::string shader_path{};
  shader_path.reserve(static_cast<uindex>(26) + resource::kMaxShaderNameLen);
  shader_path += "shaders/vulkan/";
  shader_path += resource.descr.shader_name;
  shader_path += ".vk.cshader";
  descr.shader_id = resource::GenerateResourceIdFromPath(shader_path);

  return Generate(descr);
}

Material* MaterialHandler::Get(MaterialId material_id) {
  auto* material{TryGet(material_id)};
  COMET_ASSERT(material != nullptr,
               "Requested material does not exist: ", material_id, "!");
  return material;
}

Material* MaterialHandler::TryGet(MaterialId material_id) {
  auto it{materials_.find(material_id)};

  if (it == materials_.end()) {
    return nullptr;
  }

  return &it->second;
}

Material* MaterialHandler::GetOrGenerate(const MaterialDescr& descr) {
  auto* material{TryGet(descr.id)};

  if (material != nullptr) {
    return material;
  }

  return Generate(descr);
}

Material* MaterialHandler::GetOrGenerate(
    const resource::MaterialResource& resource) {
  auto* material{TryGet(resource.id)};

  if (material != nullptr) {
    return material;
  }

  return Generate(resource);
}

void MaterialHandler::Destroy(MaterialId material_id) {
  return Destroy(*Get(material_id));
}

void MaterialHandler::Destroy(Material& material) {
  return Destroy(material, false);
}

void MaterialHandler::UpdateInstance(Material& material, ShaderId shader_id) {
  auto* shader_ptr{shader_handler_->Get(shader_id)};
  COMET_ASSERT(shader_ptr != nullptr, "Material's shader cannot be null!");
  auto& shader{*shader_ptr};
  COMET_ASSERT(shader.instance_ubo_data.uniform_count > 0 ||
                   shader.instance_ubo_data.sampler_count > 0,
               "Cannot update instance on instance-less shader (ID: ",
               COMET_STRING_ID_LABEL(shader.id), ")!");
  auto image_index{context_->GetImageIndex()};
  auto& instance{shader_handler_->GetInstance(shader, material)};
  auto instance_descriptor_set_handle{
      instance.uniform_data.descriptor_set_handles[image_index]};
  u32 descriptor_count{0};
  auto frame_count{context_->GetFrameCount()};

  if (material.instance_update_frame != frame_count) {
    shader_handler_->BindInstance(shader, material.instance_id);

    shader_handler_->SetUniform(shader, shader.uniform_indices.diffuse_map,
                                &material.diffuse_map);
    shader_handler_->SetUniform(shader, shader.uniform_indices.specular_map,
                                &material.specular_map);
    shader_handler_->SetUniform(shader, shader.uniform_indices.normal_map,
                                &material.normal_map);
    shader_handler_->SetUniform(shader, shader.uniform_indices.diffuse_color,
                                &material.diffuse_color);
    shader_handler_->SetUniform(shader, shader.uniform_indices.shininess,
                                &material.shininess);

    VkDescriptorBufferInfo buffer_info;
    std::array<VkWriteDescriptorSet, kDescriptorBindingCount>
        write_descriptor_sets{};

    // TODO(m4jr0): Check if this needs to be updated.
    if (shader.instance_ubo_data.uniform_count > 0) {
      buffer_info = init::GenerateDescriptorBufferInfo(
          shader.uniform_buffer.handle, shader.bound_ubo_offset,
          shader.instance_ubo_data.ubo_stride);
      write_descriptor_sets[descriptor_count] =
          init::GenerateBufferWriteDescriptorSet(
              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, instance_descriptor_set_handle,
              &buffer_info, descriptor_count);
      instance.uniform_data.update_frame = frame_count;
      ++descriptor_count;
    }

    auto& descriptor_set_data{
        shader.layout_bindings.list[kShaderDescriptorSetInstanceIndex]};
    auto sampler_count{
        descriptor_set_data.bindings[descriptor_set_data.sampler_binding_index]
            .descriptorCount};
    std::vector<VkDescriptorImageInfo> image_info_list(sampler_count);

    if (shader.instance_ubo_data.sampler_count > 0) {
      for (u32 i{0}; i < sampler_count; ++i) {
        auto& texture_map{instance.uniform_data.texture_maps[i]};
        const auto* texture{texture_map->texture};
        auto& image_info{image_info_list[i]};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = texture->image.image_view_handle;
        image_info.sampler = texture_map->sampler->handle;
      }

      write_descriptor_sets[descriptor_count] =
          init::GenerateImageWriteDescriptorSet(
              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
              instance_descriptor_set_handle, image_info_list.data(),
              static_cast<u32>(image_info_list.size()), descriptor_count);

      ++descriptor_count;
    }

    if (descriptor_count > 0) {
      vkUpdateDescriptorSets(context_->GetDevice(), descriptor_count,
                             write_descriptor_sets.data(), 0, VK_NULL_HANDLE);
    }
  }

  vkCmdBindDescriptorSets(context_->GetFrameData().command_buffer_handle,
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          shader.pipeline->layout_handle,
                          kShaderDescriptorSetInstanceIndex, 1,
                          &instance_descriptor_set_handle, 0, VK_NULL_HANDLE);
  material.instance_update_frame = frame_count;
}

Material* MaterialHandler::GenerateInternal(const MaterialDescr& descr) {
  Material material;
  material.id = descr.id;
  material.shader_id = descr.shader_id;
  material.diffuse_map = descr.diffuse_map;
  material.specular_map = descr.specular_map;
  material.normal_map = descr.normal_map;

  auto insert_pair{materials_.emplace(material.id, material)};
  COMET_ASSERT(insert_pair.second, "Could not insert material: ",
               COMET_STRING_ID_LABEL(material.id), "!");
  shader_handler_->BindMaterial(insert_pair.first->second);
  return &insert_pair.first->second;
}

TextureMap MaterialHandler::GenerateTextureMap(
    const resource::TextureMap& map) {
  const auto texture_id{map.texture_id != resource::kInvalidResourceId
                            ? map.texture_id
                            : resource::GetDefaultTextureFromType(map.type)};

  return TextureMap{
      GetOrGenerateSampler(map),
      texture_handler_->GetOrGenerate(
          resource_manager_->Load<resource::TextureResource>(texture_id)),
      map.type};
}

void MaterialHandler::Destroy(Material& material, bool is_destroying_handler) {
  if (shader_handler_->IsInitialized() &&
      shader_handler_->HasMaterial(material)) {
    shader_handler_->UnbindMaterial(material);
  }

  material.shininess = .0f;
  material.instance_update_frame = kInvalidFrameIndex;
  material.id = kInvalidMaterialId;
  material.instance_id = kInvalidMaterialInstanceId;
  material.shader_id = kInvalidShaderId;
  material.diffuse_color = {kColorWhite, 1.0f};

  if (!is_destroying_handler) {
    std::array<TextureMap*, 3> texture_maps = {
        &material.diffuse_map, &material.specular_map, &material.normal_map};

    for (auto* texture_map : texture_maps) {
      Destroy(*texture_map->sampler);
      *texture_map = {};
    }
    materials_.erase(material.id);
  }
}

Sampler* MaterialHandler::GenerateSampler(SamplerId sampler_id,
                                          const VkSamplerCreateInfo& info) {
  Sampler sampler{};
  sampler.id = sampler_id;
  sampler.ref_count = 0;

  COMET_CHECK_VK(vkCreateSampler(context_->GetDevice(), &info, VK_NULL_HANDLE,
                                 &sampler.handle),
                 "Failed to create texture sampler!");

  const auto insert_pair{samplers_.emplace(sampler.id, sampler)};
  COMET_ASSERT(insert_pair.second,
               "Could not insert sampler: ", COMET_STRING_ID_LABEL(sampler.id),
               "!");
  return &insert_pair.first->second;
}

Sampler* MaterialHandler::GetSampler(SamplerId sampler_id) {
  auto* sampler{TryGetSampler(sampler_id)};
  COMET_ASSERT(sampler != nullptr,
               "Requested sampler does not exist: ", sampler_id, "!");
  return sampler;
}

Sampler* MaterialHandler::TryGetSampler(SamplerId sampler_id) {
  auto it{samplers_.find(sampler_id)};

  if (it == samplers_.end()) {
    return nullptr;
  }

  return &it->second;
}

Sampler* MaterialHandler::GetOrGenerateSampler(
    const resource::TextureMap& texture_map) {
  const auto sampler_info{init::GenerateSamplerCreateInfo(
      VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, texture_map,
      context_->IsSamplerAnisotropy(),
      context_->GetDevice().GetProperties().limits.maxSamplerAnisotropy)};

  auto sampler_id{std::hash<VkSamplerCreateInfo>()(sampler_info)};
  auto* sampler{TryGetSampler(sampler_id)};

  if (sampler != nullptr) {
    ++sampler->ref_count;
    return sampler;
  }

  return GenerateSampler(sampler_id, sampler_info);
}

void MaterialHandler::Destroy(Sampler& sampler) {
  if (sampler.ref_count > 1) {
    --sampler.ref_count;
    return;
  }

  if (sampler.handle != VK_NULL_HANDLE) {
    vkDestroySampler(context_->GetDevice(), sampler.handle, VK_NULL_HANDLE);
    sampler.handle = VK_NULL_HANDLE;
  }

  samplers_.erase(sampler.id);
  sampler.id = kInvalidSamplerId;
  sampler.ref_count = 0;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet