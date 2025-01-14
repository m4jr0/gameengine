// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "vulkan_shader_handler.h"

#include <functional>
#include <utility>

#include "comet/core/c_string.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"
#include "comet/rendering/driver/vulkan/data/vulkan_buffer.h"
#include "comet/rendering/driver/vulkan/data/vulkan_descriptor.h"
#include "comet/rendering/driver/vulkan/data/vulkan_pipeline.h"
#include "comet/rendering/driver/vulkan/data/vulkan_shader_data.h"
#include "comet/rendering/driver/vulkan/data/vulkan_texture_map.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_buffer_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_common_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_descriptor_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_initializer_utils.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/resource_manager.h"

namespace comet {
namespace rendering {
namespace vk {
ShaderHandler::ShaderHandler(const ShaderHandlerDescr& descr)
    : Handler{descr},
      shader_module_handler_{descr.shader_module_handler},
      pipeline_handler_{descr.pipeline_handler},
      material_handler_{descr.material_handler},
      texture_handler_{descr.texture_handler},
      descriptor_handler_{descr.descriptor_handler} {
  COMET_ASSERT(shader_module_handler_ != nullptr,
               "Shader module handler is null!");
  COMET_ASSERT(pipeline_handler_ != nullptr, "Pipeline handler is null!");
  COMET_ASSERT(material_handler_ != nullptr, "Material handler is null!");
  COMET_ASSERT(texture_handler_ != nullptr, "Texture handler is null!");
  COMET_ASSERT(descriptor_handler_ != nullptr, "Descriptor handler is null!");
}

void ShaderHandler::Initialize() {
  Handler::Initialize();
  general_allocator_.Initialize();
  shader_instance_allocator_.Initialize();
  shaders_ = Map<ShaderId, Shader*>{&shader_instance_allocator_};
  descriptor_set_layout_count_ = context_->GetImageCount();
}

void ShaderHandler::Shutdown() {
  descriptor_set_layout_count_ = 0;

  for (auto& it : shaders_) {
    Destroy(it.value, true);
  }

  shaders_.Clear();
  bound_shader_ = nullptr;
  instance_id_handler_.Shutdown();
  shader_instance_allocator_.Destroy();
  general_allocator_.Destroy();
  Handler::Shutdown();
}

Shader* ShaderHandler::Generate(const ShaderDescr& descr) {
  const auto* shader_resource{
      resource::ResourceManager::Get().Load<resource::ShaderResource>(
          descr.resource_path)};
  COMET_ASSERT(shader_resource != nullptr, "Shader resource is null!");
  auto* shader{shader_instance_allocator_.AllocateOneAndPopulate<Shader>()};
  shader->id = shader_resource->id;
  shader->render_pass = descr.render_pass;
  shader->is_wireframe = shader_resource->descr.is_wireframe;
  shader->cull_mode = shader_resource->descr.cull_mode;

  shader->vertex_attributes =
      Array<VkVertexInputAttributeDescription>{&general_allocator_};
  shader->uniforms = Array<ShaderUniform>{&general_allocator_};
  shader->constants = Array<ShaderConstant>{&general_allocator_};
  shader->storages = Array<ShaderStorage>{&general_allocator_};
  shader->modules = Array<const ShaderModule*>{&general_allocator_};
  shader->push_constant_ranges =
      Array<VkPushConstantRange>{&general_allocator_};

  shader->global_uniform_data.descriptor_set_handles =
      Array<VkDescriptorSet>{&general_allocator_};
  shader->global_uniform_data.texture_maps =
      Array<const TextureMap*>{&general_allocator_};

  shader->storage_data.descriptor_set_handles =
      Array<VkDescriptorSet>{&general_allocator_};

  shader->instances.list = Array<MaterialInstance>{&shader_instance_allocator_};
  shader->instances.ids =
      Array<MaterialInstanceId>{&shader_instance_allocator_};

  HandleShaderModulesGeneration(shader, shader_resource);
  HandleAttributesGeneration(shader, shader_resource);
  HandleUniformsGeneration(shader, shader_resource);
  HandleConstantsGeneration(shader, shader_resource);
  HandleStorageGeneration(shader, shader_resource);
  HandleBindingsGeneration(shader);
  HandleDescriptorSetLayoutsGeneration(shader);
  HandlePipelineGeneration(shader);
  HandleDescriptorPoolGeneration(shader);
  HandleUboBufferGeneration(shader);

  return shaders_.Emplace(shader->id, shader).value;
}

Shader* ShaderHandler::Get(ShaderId shader_id) {
  auto* shader{TryGet(shader_id)};
  COMET_ASSERT(shader != nullptr, "Requested shader does not exist: ",
               COMET_STRING_ID_LABEL(shader_id), "!");
  return shader;
}

Shader* ShaderHandler::TryGet(ShaderId shader_id) {
  auto** shader{shaders_.TryGet(shader_id)};

  if (shader == nullptr) {
    return nullptr;
  }

  return *shader;
}

void ShaderHandler::Bind(const Shader* shader,
                         PipelineType pipeline_type) const {
  const Pipeline* pipeline{nullptr};

  if (pipeline_type == PipelineType::Graphics) {
    pipeline = shader->graphics_pipeline;
  } else if (pipeline_type == PipelineType::Compute) {
    pipeline = shader->compute_pipeline;
  }

  COMET_ASSERT(pipeline != nullptr, "Tried to bind an invalid pipeline!");
  pipeline_handler_->Bind(pipeline);

  auto bind_point{pipeline->type == PipelineType::Graphics
                      ? VK_PIPELINE_BIND_POINT_GRAPHICS
                      : VK_PIPELINE_BIND_POINT_COMPUTE};

  auto command_buffer_handle{context_->GetFrameData().command_buffer_handle};
  auto image_index{context_->GetImageIndex()};

  auto global_set_index{shader->descriptor_set_indices.global};
  auto storage_set_index{shader->descriptor_set_indices.storage};

  if (global_set_index != kInvalidShaderDescriptorSetIndex &&
      IsBindPoint(bind_point, shader->global_ubo_data.stages)) {
    auto set_handle{
        shader->global_uniform_data.descriptor_set_handles[image_index]};

    vkCmdBindDescriptorSets(command_buffer_handle, bind_point,
                            pipeline->layout_handle, global_set_index, 1,
                            &set_handle, 0, VK_NULL_HANDLE);
  }

  if (storage_set_index != kInvalidShaderDescriptorSetIndex &&
      IsBindPoint(bind_point, shader->storage_data.stages)) {
    auto set_handle{shader->storage_data.descriptor_set_handles[image_index]};

    vkCmdBindDescriptorSets(command_buffer_handle, bind_point,
                            pipeline->layout_handle, storage_set_index, 1,
                            &set_handle, 0, VK_NULL_HANDLE);
  }
}

void ShaderHandler::BindInstance(Shader* shader, MaterialId material_id,
                                 PipelineType pipeline_type) {
  BindInstance(shader, material_handler_->Get(material_id), pipeline_type);
}

void ShaderHandler::BindInstance(Shader* shader, const Material* material,
                                 PipelineType pipeline_type) {
  auto instance_set_index{shader->descriptor_set_indices.instance};

  if (instance_set_index == kInvalidShaderDescriptorSetIndex) {
    return;
  }

  const Pipeline* pipeline{nullptr};

  if (pipeline_type == PipelineType::Graphics) {
    pipeline = shader->graphics_pipeline;
  } else if (pipeline_type == PipelineType::Compute) {
    pipeline = shader->compute_pipeline;
  }

  COMET_ASSERT(pipeline != nullptr, "Tried to bind an invalid pipeline!");
  pipeline_handler_->Bind(pipeline);

  auto bind_point{pipeline->type == PipelineType::Graphics
                      ? VK_PIPELINE_BIND_POINT_GRAPHICS
                      : VK_PIPELINE_BIND_POINT_COMPUTE};

  if (shader->instance_ubo_data.ubo_size > 0 &&
      IsBindPoint(bind_point, shader->instance_ubo_data.stages)) {
    auto command_buffer_handle{context_->GetFrameData().command_buffer_handle};
    auto image_index{context_->GetImageIndex()};
    auto& instance{GetInstance(shader, material)};
    auto set_handle{instance.uniform_data.descriptor_set_handles[image_index]};

    vkCmdBindDescriptorSets(command_buffer_handle, bind_point,
                            pipeline->layout_handle, instance_set_index, 1,
                            &set_handle, 0, VK_NULL_HANDLE);
  }
}

void ShaderHandler::Destroy(ShaderId shader_id) { Destroy(Get(shader_id)); }

void ShaderHandler::Destroy(Shader* shader) { Destroy(shader, false); }

void ShaderHandler::Reset() { bound_shader_ = nullptr; }

void ShaderHandler::UpdateGlobals(Shader* shader,
                                  const frame::FramePacket* packet) const {
  UpdateGlobals(shader, {&packet->projection_matrix, &packet->view_matrix});
}

void ShaderHandler::UpdateGlobals(Shader* shader,
                                  const ShaderGlobalsUpdate& update) const {
  auto global_set_index{shader->descriptor_set_indices.global};

  if (global_set_index == kInvalidShaderDescriptorSetIndex) {
    return;
  }

  auto frame_count{context_->GetFrameCount()};

  if (shader->global_uniform_data.update_frame == frame_count) {
    return;
  }

  shader->bound_ubo_offset = shader->global_ubo_data.ubo_offset;
  SetUniform(shader, shader->uniform_indices.projection,
             update.projection_matrix);
  SetUniform(shader, shader->uniform_indices.view, update.view_matrix);

  auto global_descriptor_set_handle{
      shader->global_uniform_data
          .descriptor_set_handles[context_->GetImageIndex()]};

  auto buffer_info{init::GenerateDescriptorBufferInfo(
      shader->uniform_buffer.handle, shader->global_ubo_data.ubo_offset,
      shader->global_ubo_data.ubo_stride)};

  auto ubo_write_descriptor_set{init::GenerateBufferWriteDescriptorSet(
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, global_descriptor_set_handle,
      &buffer_info, 0)};
  const auto binding_count{
      shader->layout_bindings.list[global_set_index].binding_count};

  vkUpdateDescriptorSets(context_->GetDevice(), binding_count,
                         &ubo_write_descriptor_set, 0, VK_NULL_HANDLE);
  shader->global_uniform_data.update_frame = frame_count;
}

void ShaderHandler::UpdateConstants(Shader* shader,
                                    const frame::FramePacket* packet) const {
  auto draw_count{static_cast<u32>(packet->draw_count)};
  UpdateConstants(shader, {&draw_count, nullptr});
}

void ShaderHandler::UpdateConstants(Shader* shader,
                                    const ShaderConstantsUpdate& update) const {
  if (update.draw_count != nullptr) {
    SetConstant(shader, shader->constant_indices.draw_count, update.draw_count);
  }

  if (update.count != nullptr) {
    SetConstant(shader, shader->constant_indices.count, update.count);
  }
}

void ShaderHandler::UpdateStorages(Shader* shader,
                                   const frame::FramePacket* packet) const {
  if (packet->rendering_data == nullptr) {
    return;
  }

  const auto* update{
      static_cast<const ShaderStoragesUpdate*>(packet->rendering_data)};
  UpdateStorages(shader, *update);
}

void ShaderHandler::UpdateStorages(Shader* shader,
                                   const ShaderStoragesUpdate& update) const {
  auto storage_set_index{shader->descriptor_set_indices.storage};

  if (storage_set_index == kInvalidShaderDescriptorSetIndex) {
    return;
  }

  const auto binding_count{
      shader->layout_bindings.list[storage_set_index].binding_count};

  if (binding_count == 0) {
    return;
  }

  frame::FrameArray<VkDescriptorSetLayout> layout_buffer{};
  layout_buffer.Reserve(descriptor_set_layout_count_);

  for (u32 i{0}; i < descriptor_set_layout_count_; ++i) {
    layout_buffer.PushBack(shader->layout_handles[storage_set_index]);
  }

  shader->storage_data.descriptor_set_handles.Resize(
      descriptor_set_layout_count_);

  auto is_allocated{descriptor_handler_->Generate(
      layout_buffer.GetData(),
      shader->storage_data.descriptor_set_handles.GetData(),
      descriptor_set_layout_count_, DescriptorType::Dynamic)};

  COMET_ASSERT(is_allocated,
               "Failed to allocate spatial uniform descriptor sets!");

  VkDescriptorSet set_handle{
      shader->storage_data.descriptor_set_handles[context_->GetImageIndex()]};

  frame::FrameArray<VkDescriptorBufferInfo> buffer_info{};
  frame::FrameArray<VkDescriptorSetLayoutBinding> bindings{};
  frame::FrameArray<VkWriteDescriptorSet> write_descriptor_sets{};

  buffer_info.Reserve(binding_count);
  bindings.Reserve(binding_count);
  write_descriptor_sets.Reserve(binding_count);

  BindStorageBuffer(
      shader, set_handle, shader->storage_indices.proxy_local_datas,
      update.ssbo_proxy_local_data_handle, update.ssbo_proxy_local_data_size,
      buffer_info, bindings, write_descriptor_sets);

  BindStorageBuffer(shader, set_handle, shader->storage_indices.proxy_ids,
                    update.ssbo_proxy_local_data_ids_handle,
                    update.ssbo_proxy_local_data_ids_size, buffer_info,
                    bindings, write_descriptor_sets);

  BindStorageBuffer(shader, set_handle, shader->storage_indices.proxy_instances,
                    update.ssbo_proxy_instances_handle,
                    update.ssbo_proxy_instances_size, buffer_info, bindings,
                    write_descriptor_sets);

  BindStorageBuffer(
      shader, set_handle, shader->storage_indices.indirect_proxies,
      update.ssbo_indirect_proxies_handle, update.ssbo_indirect_proxies_size,
      buffer_info, bindings, write_descriptor_sets);

  BindStorageBuffer(shader, set_handle, shader->storage_indices.word_indices,
                    update.ssbo_word_indices_handle,
                    update.ssbo_word_indices_size, buffer_info, bindings,
                    write_descriptor_sets);

  BindStorageBuffer(shader, set_handle, shader->storage_indices.source_words,
                    update.ssbo_source_words_handle,
                    update.ssbo_source_words_size, buffer_info, bindings,
                    write_descriptor_sets);

  BindStorageBuffer(
      shader, set_handle, shader->storage_indices.destination_words,
      update.ssbo_destination_words_handle, update.ssbo_destination_words_size,
      buffer_info, bindings, write_descriptor_sets);

  vkUpdateDescriptorSets(context_->GetDevice(),
                         static_cast<u32>(write_descriptor_sets.GetSize()),
                         write_descriptor_sets.GetData(), 0, VK_NULL_HANDLE);
}

void ShaderHandler::UpdateInstance(Shader* shader, MaterialId material_id) {
  UpdateInstance(shader, material_handler_->Get(material_id));
}

void ShaderHandler::UpdateInstance(Shader* shader, Material* material) {
  auto instance_set_index{shader->descriptor_set_indices.instance};

  if (instance_set_index == kInvalidShaderDescriptorSetIndex) {
    return;
  }

  auto image_index{context_->GetImageIndex()};
  auto& instance{GetInstance(shader, material)};
  auto instance_descriptor_set_handle{
      instance.uniform_data.descriptor_set_handles[image_index]};
  u32 descriptor_count{0};
  auto frame_count{context_->GetFrameCount()};

  if (material->instance_update_frame != frame_count) {
    shader->bound_instance_index =
        GetInstanceIndex(shader, material->instance_id);
    shader->bound_ubo_offset =
        shader->global_ubo_data.ubo_stride +
        shader->instances.list[shader->bound_instance_index].offset;

    SetUniform(shader, shader->uniform_indices.diffuse_map,
               &material->diffuse_map);
    SetUniform(shader, shader->uniform_indices.specular_map,
               &material->specular_map);
    SetUniform(shader, shader->uniform_indices.normal_map,
               &material->normal_map);
    SetUniform(shader, shader->uniform_indices.diffuse_color,
               &material->diffuse_color);
    SetUniform(shader, shader->uniform_indices.shininess, &material->shininess);

    VkDescriptorBufferInfo buffer_info;
    StaticArray<VkWriteDescriptorSet, kDescriptorBindingCount>
        write_descriptor_sets{};

    if (shader->instance_ubo_data.uniform_count > 0) {
      buffer_info = init::GenerateDescriptorBufferInfo(
          shader->uniform_buffer.handle, shader->bound_ubo_offset,
          shader->instance_ubo_data.ubo_stride);
      write_descriptor_sets[descriptor_count] =
          init::GenerateBufferWriteDescriptorSet(
              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, instance_descriptor_set_handle,
              &buffer_info, descriptor_count);
      instance.uniform_data.update_frame = frame_count;
      ++descriptor_count;
    }

    auto& descriptor_set_data{shader->layout_bindings.list[instance_set_index]};
    auto sampler_count{
        descriptor_set_data.bindings[descriptor_set_data.sampler_binding_index]
            .descriptorCount};
    frame::FrameArray<VkDescriptorImageInfo> image_info_list{};
    image_info_list.Resize(sampler_count);

    if (shader->instance_ubo_data.sampler_count > 0) {
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
              instance_descriptor_set_handle, image_info_list.GetData(),
              static_cast<u32>(image_info_list.GetSize()), descriptor_count);

      ++descriptor_count;
    }

    if (descriptor_count > 0) {
      vkUpdateDescriptorSets(context_->GetDevice(), descriptor_count,
                             write_descriptor_sets.GetData(), 0,
                             VK_NULL_HANDLE);
    }
  }

  material->instance_update_frame = frame_count;
}

void ShaderHandler::SetUniform(Shader* shader, const ShaderUniform& uniform,
                               const void* value) const {
  if (uniform.type == ShaderVariableType::Sampler) {
    if (uniform.scope == ShaderUniformScope::Global) {
      shader->global_uniform_data.texture_maps[uniform.location] =
          static_cast<const TextureMap*>(value);
    } else {
      shader->instances.list[shader->bound_instance_index]
          .uniform_data.texture_maps[uniform.location] =
          static_cast<const TextureMap*>(value);
    }

    return;
  }

  auto& buffer{shader->uniform_buffer};
  MapBuffer(buffer);
  CopyToBuffer(buffer, value, uniform.size,
               shader->bound_ubo_offset + uniform.offset);
  UnmapBuffer(buffer);
}

void ShaderHandler::SetUniform(ShaderId id, const ShaderUniform& uniform,
                               const void* value) {
  auto* shader{Get(id)};
  COMET_ASSERT(shader != nullptr, "Tried to retrieve shader pass from ID ", id,
               ", but instance is null!");
  SetUniform(shader, uniform, value);
}

void ShaderHandler::SetUniform(Shader* shader, ShaderUniformIndex index,
                               const void* value) const {
  COMET_ASSERT(index < shader->uniforms.GetSize(),
               "Tried to set uniform at index #", index,
               " for shader pass, but uniform count is ",
               shader->uniforms.GetSize(), "!");
  SetUniform(shader, shader->uniforms[index], value);
}

void ShaderHandler::SetUniform(ShaderId id, ShaderUniformIndex index,
                               const void* value) {
  auto* shader{Get(id)};
  COMET_ASSERT(shader != nullptr, "Tried to retrieve shader pass from ID ", id,
               ", but instance is null!");
  SetUniform(shader, index, value);
}

void ShaderHandler::SetConstant(Shader* shader, const ShaderConstant& constant,
                                const void* value) const {
  auto command_buffer_handle{context_->GetFrameData().command_buffer_handle};

  if (IsGraphicsStage(constant.stages)) {
    COMET_ASSERT(
        shader->graphics_pipeline != nullptr,
        "Tried to push constants to a non-existing graphics pipeline!");

    vkCmdPushConstants(command_buffer_handle,
                       shader->graphics_pipeline->layout_handle,
                       constant.stages, constant.offset, constant.size, value);
  }

  if (IsComputeStage(constant.stages)) {
    COMET_ASSERT(shader->compute_pipeline != nullptr,
                 "Tried to push constant to a non-existing compute pipeline!");

    vkCmdPushConstants(command_buffer_handle,
                       shader->compute_pipeline->layout_handle, constant.stages,
                       constant.offset, constant.size, value);
  }
}

void ShaderHandler::SetConstant(ShaderId id, const ShaderConstant& constant,
                                const void* value) {
  auto* shader{Get(id)};
  COMET_ASSERT(shader != nullptr, "Tried to retrieve shader pass from ID ", id,
               ", but instance is null!");
  SetConstant(shader, constant, value);
}

void ShaderHandler::SetConstant(Shader* shader, ShaderConstantIndex index,
                                const void* value) const {
  COMET_ASSERT(index < shader->constants.GetSize(),
               "Tried to set constant at index #", index,
               " for shader pass, but constant count is ",
               shader->constants.GetSize(), "!");
  SetConstant(shader, shader->constants[index], value);
}

void ShaderHandler::SetConstant(ShaderId id, ShaderConstantIndex index,
                                const void* value) {
  auto* shader{Get(id)};
  COMET_ASSERT(shader != nullptr, "Tried to retrieve shader pass from ID ", id,
               ", but instance is null!");
  SetUniform(shader, index, value);
}

void ShaderHandler::BindMaterial(Material* material) {
  auto* shader{Get(material->shader_id)};

  if (shader == nullptr) {
    return;
  }

  auto instance_set_index{shader->descriptor_set_indices.instance};

  if (instance_set_index == kInvalidShaderDescriptorSetIndex) {
    return;
  }

  MaterialInstance instance{};
  auto& descriptor_set{shader->layout_bindings.list[instance_set_index]};
  auto sampler_binding_index{descriptor_set.sampler_binding_index};
  auto instance_texture_count{
      descriptor_set.bindings[sampler_binding_index].descriptorCount};

  instance.uniform_data.descriptor_set_handles =
      Array<VkDescriptorSet>{&general_allocator_};
  instance.uniform_data.texture_maps =
      Array<const TextureMap*>{&general_allocator_};

  if (instance_texture_count > 0) {
    instance.uniform_data.texture_maps.Resize(instance_texture_count);

    // TODO(m4jr0): Put texture maps in generic array.
    StaticArray<TextureMap*, 3> maps{
        &material->diffuse_map, &material->specular_map, &material->normal_map};
    memory::CopyMemory(instance.uniform_data.texture_maps.GetData(),
                       maps.GetData(), sizeof(TextureMap*) * maps.GetSize());
  }

  frame::FrameArray<VkDescriptorSetLayout> layout_buffer{};
  layout_buffer.Reserve(descriptor_set_layout_count_);
  instance.uniform_data.descriptor_set_handles.Resize(
      descriptor_set_layout_count_);
  instance.uniform_data.descriptor_pool_handle = shader->descriptor_pool_handle;

  for (u32 i{0}; i < descriptor_set_layout_count_; ++i) {
    layout_buffer.PushBack(shader->layout_handles[instance_set_index]);
  }

  auto is_allocated{descriptor_handler_->Generate(
      layout_buffer.GetData(),
      instance.uniform_data.descriptor_set_handles.GetData(),
      descriptor_set_layout_count_, DescriptorType::Static)};

  COMET_ASSERT(is_allocated,
               "Failed to allocate instance uniform descriptor sets!");

  instance.offset =
      shader->instance_ubo_data.ubo_stride * shader->instances.list.GetSize();

  // Generate material instance.
  shader->instances.list.EmplaceBack(std::move(instance));
  material->instance_id = instance_id_handler_.Generate();
  instance.id = material->instance_id;
  shader->instances.ids.PushBack(material->instance_id);
}

void ShaderHandler::UnbindMaterial(Material* material) {
  COMET_ASSERT(HasMaterial(material),
               "Trying to destroy dead material instance #",
               material->instance_id, "!");

  auto* shader{Get(material->shader_id)};
  auto index{GetInstanceIndex(shader, material)};
  auto& instances{shader->instances.list};
  auto& instance{instances[index]};
  auto& device{context_->GetDevice()};

  auto& uniform_data{instance.uniform_data};
  FreeDescriptor(device, uniform_data.descriptor_set_handles,
                 uniform_data.descriptor_pool_handle);
  instance.uniform_data.texture_maps.Clear();

  // Delete material instance.
  // Copy last instance in place of the deleted one + Remove last duplicate in
  // the list.
  auto& instance_ids{shader->instances.ids};
  COMET_ASSERT(instance_ids.GetSize() == instances.GetSize(),
               "Size mismatch between material IDs (", instance_ids.GetSize(),
               ") and instances (", instances.GetSize(),
               ")! Something very wrong must have happened!");
  auto max_index{instance_ids.GetSize() - 1};

  if (index != max_index) {
    instance_ids[index] = instance_ids[max_index];
    instances[index] = instances[max_index];
  }

  instance_ids.Resize(max_index);
  instances.Resize(max_index);
  instance_id_handler_.Destroy(material->instance_id);
  material->instance_id = kInvalidMaterialInstanceId;
}

bool ShaderHandler::HasMaterial(const Material* material) const {
  return instance_id_handler_.IsAlive(material->instance_id);
}

MaterialInstance& ShaderHandler::GetInstance(Shader* shader,
                                             const Material* material) {
  return shader->instances.list[GetInstanceIndex(shader, material)];
}

ShaderVertexAttributeSize ShaderHandler::GetVertexAttributeSize(
    ShaderVertexAttributeType type) {
  switch (type) {
    case ShaderVertexAttributeType::F16:
      return 2;
    case ShaderVertexAttributeType::F32:
      return 4;
    case ShaderVertexAttributeType::F64:
      return 8;
    case ShaderVertexAttributeType::Vec2:
      return 8;
    case ShaderVertexAttributeType::Vec3:
      return 12;
    case ShaderVertexAttributeType::Vec4:
      return 16;
    case ShaderVertexAttributeType::S8:
      return 1;
    case ShaderVertexAttributeType::S16:
      return 2;
    case ShaderVertexAttributeType::S32:
      return 4;
    case ShaderVertexAttributeType::U8:
      return 1;
    case ShaderVertexAttributeType::U16:
      return 2;
    case ShaderVertexAttributeType::U32:
      return 4;

    default:
      COMET_ASSERT(
          false, "Unknown or unsupported shader vertex attribute type: ",
          static_cast<std::underlying_type_t<ShaderVariableType>>(type), "!");
  }

  return kInvalidShaderVertexAttributeSize;
}

ShaderUniformSize ShaderHandler::ShaderVariableTypeSize(
    ShaderVariableType type) {
  switch (type) {
    case ShaderVariableType::B32:
    case ShaderVariableType::S32:
    case ShaderVariableType::U32:
    case ShaderVariableType::F32:
      return 4;

    case ShaderVariableType::F64:
      return 8;

    case ShaderVariableType::B32Vec2:
    case ShaderVariableType::S32Vec2:
    case ShaderVariableType::U32Vec2:
    case ShaderVariableType::Vec2:
      return 8;

    case ShaderVariableType::B32Vec3:
    case ShaderVariableType::S32Vec3:
    case ShaderVariableType::U32Vec3:
    case ShaderVariableType::Vec3:
      return 12;

    case ShaderVariableType::B32Vec4:
    case ShaderVariableType::S32Vec4:
    case ShaderVariableType::U32Vec4:
    case ShaderVariableType::Vec4:
      return 16;

    case ShaderVariableType::F64Vec2:
      return 16;

    case ShaderVariableType::F64Vec3:
      return 24;

    case ShaderVariableType::F64Vec4:
      return 32;

    case ShaderVariableType::Mat2x2:
      return 8;

    case ShaderVariableType::Mat2x3:
    case ShaderVariableType::Mat3x2:
      return 24;

    case ShaderVariableType::Mat3x3:
      return 36;

    case ShaderVariableType::Mat2x4:
    case ShaderVariableType::Mat4x2:
      return 32;

    case ShaderVariableType::Mat3x4:
    case ShaderVariableType::Mat4x3:
      return 48;

    case ShaderVariableType::Mat4x4:
      return 64;

    case ShaderVariableType::Sampler:
    case ShaderVariableType::Image:
    case ShaderVariableType::Atomic:
      return kInvalidShaderUniformSize;

    default:
      COMET_ASSERT(
          false, "Unknown or unsupported shader data type: ",
          static_cast<std::underlying_type_t<ShaderVariableType>>(type), "!");
  }

  return kInvalidShaderUniformSize;
}

VkFormat ShaderHandler::GetVkFormat(ShaderVertexAttributeType type) {
  switch (type) {
    case ShaderVertexAttributeType::F16:
      return VK_FORMAT_R16_SFLOAT;
    case ShaderVertexAttributeType::F32:
      return VK_FORMAT_R32_SFLOAT;
    case ShaderVertexAttributeType::F64:
      return VK_FORMAT_R64_SFLOAT;
    case ShaderVertexAttributeType::Vec2:
      return VK_FORMAT_R32G32_SFLOAT;
    case ShaderVertexAttributeType::Vec3:
      return VK_FORMAT_R32G32B32_SFLOAT;
    case ShaderVertexAttributeType::Vec4:
      return VK_FORMAT_R32G32B32A32_SFLOAT;
    case ShaderVertexAttributeType::S8:
      return VK_FORMAT_R8_SINT;
    case ShaderVertexAttributeType::S16:
      return VK_FORMAT_R16_SINT;
    case ShaderVertexAttributeType::S32:
      return VK_FORMAT_R32_SINT;
    case ShaderVertexAttributeType::U8:
      return VK_FORMAT_R8_UINT;
    case ShaderVertexAttributeType::U16:
      return VK_FORMAT_R16_UINT;
    case ShaderVertexAttributeType::U32:
      return VK_FORMAT_R32_UINT;

    default:
      COMET_ASSERT(
          false, "Unknown or unsupported shader vertex attribute type: ",
          static_cast<std::underlying_type_t<ShaderVariableType>>(type), "!");
  }

  return VK_FORMAT_UNDEFINED;
}

void ShaderHandler::BindStorageBuffer(
    const Shader* shader, VkDescriptorSet set_handle,
    ShaderStorageIndex storage_index, VkBuffer buffer_handle,
    VkDeviceSize buffer_size, Array<VkDescriptorBufferInfo>& buffer_info,
    Array<VkDescriptorSetLayoutBinding>& bindings,
    Array<VkWriteDescriptorSet>& write_descriptor_sets) {
  if (storage_index == kInvalidShaderStorageIndex ||
      buffer_handle == VK_NULL_HANDLE) {
    return;
  }

  buffer_info.EmplaceBack(
      init::GenerateDescriptorBufferInfo(buffer_handle, 0, buffer_size));

  bindings.EmplaceBack(init::GenerateDescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, shader->storages[storage_index].stages,
      storage_index));

  write_descriptor_sets.EmplaceBack(init::GenerateBufferWriteDescriptorSet(
      VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, set_handle, &buffer_info.GetLast(),
      storage_index));
}

void ShaderHandler::Destroy(Shader* shader, bool is_destroying_handler) {
  const auto& device{context_->GetDevice()};

  shader->global_uniform_data = {};
  shader->storage_data = {};
  shader->global_ubo_data = {};
  shader->instance_ubo_data = {};

  if (IsBufferInitialized(shader->uniform_buffer)) {
    DestroyBuffer(shader->uniform_buffer);
  }

  if (shader->descriptor_pool_handle != VK_NULL_HANDLE) {
    DestroyDescriptorPool(device, shader->descriptor_pool_handle);
  }

  shader->instances.ids.Clear();
  shader->instances.list.Clear();

  for (u32 i{0}; i < shader->layout_bindings.count; ++i) {
    auto layout_handle{shader->layout_handles[i]};

    if (layout_handle == VK_NULL_HANDLE) {
      continue;
    }

    vkDestroyDescriptorSetLayout(device, layout_handle, VK_NULL_HANDLE);
    shader->layout_handles[i] = VK_NULL_HANDLE;
  }

  shader->layout_bindings = {};
  shader->vertex_attributes.Clear();
  shader->uniforms.Clear();
  shader->constants.Clear();
  shader->storages.Clear();

  if (shader_module_handler_->IsInitialized()) {
    for (auto& module : shader->modules) {
      shader_module_handler_->Destroy(module->id);
    }
  }

  shader->modules.Clear();
  shader->push_constant_ranges.Clear();

  if (!is_destroying_handler) {
    shaders_.Remove(shader->id);
  }

  shader_instance_allocator_.Deallocate(shader);
}

u32 ShaderHandler::GetInstanceIndex(const Shader* shader,
                                    const Material* material) const {
  return GetInstanceIndex(shader, material->instance_id);
}

u32 ShaderHandler::GetInstanceIndex(
    const Shader* shader, const MaterialInstanceId instance_id) const {
  auto& instance_ids{shader->instances.ids};
  auto it{std::find(instance_ids.begin(), instance_ids.end(), instance_id)};
  COMET_ASSERT(it != instance_ids.end(),
               "Unable to find bound material instance (ID: ", instance_id,
               ") in shader ", COMET_STRING_ID_LABEL(shader->id), "!");
  return static_cast<u32>(it - instance_ids.begin());
}

void ShaderHandler::HandleShaderModulesGeneration(
    Shader* shader, const resource::ShaderResource* resource) const {
  shader->modules.Reserve(resource->descr.shader_module_paths.GetSize());

  for (const auto& shader_module_path : resource->descr.shader_module_paths) {
    shader->modules.PushBack(
        shader_module_handler_->GetOrGenerate(shader_module_path));
  }
}

void ShaderHandler::HandleAttributesGeneration(
    Shader* shader, const resource::ShaderResource* resource) const {
  auto vertex_attribute_count{
      static_cast<u32>(resource->descr.vertex_attributes.GetSize())};
  u32 offset{0};
  shader->vertex_attributes.Reserve(vertex_attribute_count);

  for (u32 i{0}; i < resource->descr.vertex_attributes.GetSize(); ++i) {
    const auto& vertex_attribute_descr{resource->descr.vertex_attributes[i]};
    VkVertexInputAttributeDescription vertex_attribute{};
    vertex_attribute.location = i;
    vertex_attribute.binding = 0;
    vertex_attribute.offset = offset;
    vertex_attribute.format = GetVkFormat(vertex_attribute_descr.type);
    offset += GetVertexAttributeSize(vertex_attribute_descr.type);
    shader->vertex_attributes.PushBack(vertex_attribute);
  }

  shader->vertex_attribute_stride = offset;
}

void ShaderHandler::HandleUniformsGeneration(
    Shader* shader, const resource::ShaderResource* resource) const {
  u32 instance_texture_count{0};

  for (const auto& uniform_descr : resource->descr.uniforms) {
    if (uniform_descr.type == ShaderVariableType::Sampler) {
      HandleSamplerGeneration(shader, uniform_descr, instance_texture_count);
      continue;
    }

    AddUniform(shader, uniform_descr, 0);
  }
}

void ShaderHandler::HandleConstantsGeneration(
    Shader* shader, const resource::ShaderResource* resource) const {
  for (const auto& constant_descr : resource->descr.constants) {
    AddConstant(shader, constant_descr);
  }
}

void ShaderHandler::HandleStorageGeneration(
    Shader* shader, const resource::ShaderResource* resource) const {
  for (const auto& storage_descr : resource->descr.storages) {
    AddStorage(shader, storage_descr);
  }
}

void ShaderHandler::HandleBindingsGeneration(Shader* shader) const {
  shader->global_ubo_data.uniform_count = 0;
  shader->global_ubo_data.sampler_count = 0;
  shader->instance_ubo_data.uniform_count = 0;
  shader->instance_ubo_data.sampler_count = 0;
  shader->storage_data.count = 0;

  for (const auto& uniform : shader->uniforms) {
    if (uniform.scope == ShaderUniformScope::Global) {
      if (uniform.type == ShaderVariableType::Sampler) {
        ++shader->global_ubo_data.sampler_count;
      } else {
        ++shader->global_ubo_data.uniform_count;
      }
    } else if (uniform.scope == ShaderUniformScope::Instance) {
      if (uniform.type == ShaderVariableType::Sampler) {
        ++shader->instance_ubo_data.sampler_count;
      } else {
        ++shader->instance_ubo_data.uniform_count;
      }
    } else {
      COMET_ASSERT(false, "Unknown or unsupported uniform scope: ",
                   static_cast<std::underlying_type_t<ShaderUniformScope>>(
                       uniform.scope),
                   "!");
    }
  }

  ShaderDescriptorSetIndex descriptor_set_index{0};

  if (shader->global_ubo_data.uniform_count > 0 ||
      shader->global_ubo_data.sampler_count > 0) {
    shader->descriptor_set_indices.global = descriptor_set_index++;

    auto& descriptor_set_data{
        shader->layout_bindings.list[shader->descriptor_set_indices.global]};

    if (shader->global_ubo_data.uniform_count > 0) {
      auto& descriptor_set{
          descriptor_set_data.bindings[descriptor_set_data.binding_count]};
      descriptor_set.binding = descriptor_set_data.binding_count;
      descriptor_set.descriptorCount = 1;
      descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptor_set.stageFlags = shader->global_ubo_data.stages;
      ++descriptor_set_data.binding_count;
    }

    if (shader->global_ubo_data.sampler_count > 0) {
      auto& descriptor_set{
          descriptor_set_data.bindings[shader->descriptor_set_indices.global]};

      descriptor_set.binding = descriptor_set_data.binding_count;
      descriptor_set.descriptorCount = shader->global_ubo_data.sampler_count;
      descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      descriptor_set.stageFlags = shader->global_ubo_data.stages;
      descriptor_set_data.sampler_binding_index =
          descriptor_set_data.binding_count;
      ++descriptor_set_data.binding_count;
    }

    ++shader->layout_bindings.count;
  }

  if (shader->instance_ubo_data.uniform_count > 0 ||
      shader->instance_ubo_data.sampler_count > 0) {
    shader->descriptor_set_indices.instance = descriptor_set_index++;

    auto& descriptor_set_data{
        shader->layout_bindings.list[shader->descriptor_set_indices.instance]};

    if (shader->instance_ubo_data.uniform_count > 0) {
      auto& descriptor_set{
          descriptor_set_data.bindings[descriptor_set_data.binding_count]};
      descriptor_set.binding = descriptor_set_data.binding_count;
      descriptor_set.descriptorCount = 1;
      descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptor_set.stageFlags = shader->instance_ubo_data.stages;
      ++descriptor_set_data.binding_count;
    }

    if (shader->instance_ubo_data.sampler_count > 0) {
      auto& descriptor_set{
          descriptor_set_data.bindings[descriptor_set_data.binding_count]};

      descriptor_set.binding = descriptor_set_data.binding_count;
      descriptor_set.descriptorCount = shader->instance_ubo_data.sampler_count;
      descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      descriptor_set.stageFlags = shader->instance_ubo_data.stages;
      descriptor_set_data.sampler_binding_index =
          descriptor_set_data.binding_count;
      ++descriptor_set_data.binding_count;
    }

    ++shader->layout_bindings.count;
  }

  shader->storage_data.count = static_cast<u32>(shader->storages.GetSize());

  if (shader->storage_data.count > 0) {
    shader->descriptor_set_indices.storage = descriptor_set_index++;

    auto& descriptor_set_data{
        shader->layout_bindings.list[shader->descriptor_set_indices.storage]};

    for (usize binding{0}; binding < shader->storage_data.count; ++binding) {
      auto& descriptor_set{
          descriptor_set_data.bindings[descriptor_set_data.binding_count]};

      descriptor_set.binding = static_cast<u32>(binding);
      descriptor_set.descriptorCount = 1;
      descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      descriptor_set.stageFlags = shader->storage_data.stages;
      ++descriptor_set_data.binding_count;
    }

    ++shader->layout_bindings.count;
  }
}

void ShaderHandler::HandleSamplerGeneration(
    Shader* shader, const ShaderUniformDescr& uniform_descr,
    u32& instance_texture_count) const {
  auto location{kInvalidShaderUniformLocation};

  if (uniform_descr.scope == ShaderUniformScope::Global) {
    COMET_ASSERT(shader->global_uniform_data.texture_maps.GetSize() ==
                     kMaxShaderTextureMapCount,
                 "Max texture map count reached: ", kMaxShaderTextureMapCount,
                 " for shader ", COMET_STRING_ID_LABEL(shader->id), "!");

    location = static_cast<ShaderUniformLocation>(
        shader->global_uniform_data.texture_maps.GetSize() - 1);
    shader->global_uniform_data.texture_maps.Resize(
        shader->global_uniform_data.texture_maps.GetSize() + 1);
  } else {
    COMET_ASSERT(instance_texture_count <= kMaxShaderTextureMapCount,
                 "Max texture map count reached: ", kMaxShaderTextureMapCount,
                 " for shader ", COMET_STRING_ID_LABEL(shader->id), "!");
    location = static_cast<ShaderUniformLocation>(instance_texture_count++);
  }

  COMET_ASSERT(location != kInvalidShaderUniformLocation,
               "Location found is invalid for shader ",
               COMET_STRING_ID_LABEL(shader->id), "!");
  AddUniform(shader, uniform_descr, location);
}

ShaderUniformIndex ShaderHandler::HandleUniformIndex(
    Shader* shader, const ShaderUniformDescr& uniform_descr) const {
  ShaderUniformIndex* index{nullptr};

  if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                      kUniformNameProjection.data(),
                      kUniformNameProjection.size())) {
    index = &shader->uniform_indices.projection;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             kUniformNameView.data(),
                             kUniformNameView.size())) {
    index = &shader->uniform_indices.view;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             kUniformNameAmbientColor.data(),
                             kUniformNameAmbientColor.size())) {
    index = &shader->uniform_indices.ambient_color;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             kUniformNameViewPos.data(),
                             kUniformNameViewPos.size())) {
    index = &shader->uniform_indices.view_pos;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             kUniformNameDiffuseColor.data(),
                             kUniformNameDiffuseColor.size())) {
    index = &shader->uniform_indices.diffuse_color;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             kUniformNameDiffuseMap.data(),
                             kUniformNameDiffuseMap.size())) {
    index = &shader->uniform_indices.diffuse_map;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             kUniformNameSpecularMap.data(),
                             kUniformNameSpecularMap.size())) {
    index = &shader->uniform_indices.specular_map;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             kUniformNameNormalMap.data(),
                             kUniformNameNormalMap.size())) {
    index = &shader->uniform_indices.normal_map;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             kUniformNameShininess.data(),
                             kUniformNameShininess.size())) {
    index = &shader->uniform_indices.shininess;
  }

  COMET_ASSERT(index != nullptr, "Unable to find uniform index with name \"",
               uniform_descr.name, "\"!");
  COMET_ASSERT(*index == kInvalidShaderUniformIndex,
               "Uniform index with name \"", uniform_descr.name,
               "\" was already bound!");
  *index = static_cast<ShaderUniformIndex>(shader->uniforms.GetSize());
  return *index;
}

ShaderConstantIndex ShaderHandler::HandleConstantIndex(
    Shader* shader, const ShaderConstantDescr& constant_descr) const {
  ShaderConstantIndex* index{nullptr};

  if (AreStringsEqual(constant_descr.name, constant_descr.name_len,
                      kConstantNameDrawCount.data(),
                      kConstantNameDrawCount.size())) {
    index = &shader->constant_indices.draw_count;
  } else if (AreStringsEqual(constant_descr.name, constant_descr.name_len,
                             kConstantNameCount.data(),
                             kConstantNameCount.size())) {
    index = &shader->constant_indices.count;
  }

  COMET_ASSERT(index != nullptr, "Unable to find constant index with name \"",
               constant_descr.name, "\"!");
  COMET_ASSERT(*index == kInvalidShaderConstantIndex,
               "Uniform index with name \"", constant_descr.name,
               "\" was already bound!");
  *index = static_cast<ShaderUniformIndex>(shader->constants.GetSize());
  return *index;
}

ShaderStorageIndex ShaderHandler::HandleStorageIndex(
    Shader* shader, const ShaderStorageDescr& storage_descr) const {
  ShaderStorageIndex* index{nullptr};

  if (AreStringsEqual(storage_descr.name, storage_descr.name_len,
                      kStorageNameProxyLocalDatas.data(),
                      kStorageNameProxyLocalDatas.size())) {
    index = &shader->storage_indices.proxy_local_datas;
  } else if (AreStringsEqual(storage_descr.name, storage_descr.name_len,
                             kStorageNameProxyIds.data(),
                             kStorageNameProxyIds.size())) {
    index = &shader->storage_indices.proxy_ids;
  } else if (AreStringsEqual(storage_descr.name, storage_descr.name_len,
                             kStorageNameProxyInstances.data(),
                             kStorageNameProxyInstances.size())) {
    index = &shader->storage_indices.proxy_instances;
  } else if (AreStringsEqual(storage_descr.name, storage_descr.name_len,
                             kStorageNameIndirectProxies.data(),
                             kStorageNameIndirectProxies.size())) {
    index = &shader->storage_indices.indirect_proxies;
  } else if (AreStringsEqual(storage_descr.name, storage_descr.name_len,
                             kStorageNameWordIndices.data(),
                             kStorageNameWordIndices.size())) {
    index = &shader->storage_indices.word_indices;
  } else if (AreStringsEqual(storage_descr.name, storage_descr.name_len,
                             kStorageNameSourceWords.data(),
                             kStorageNameSourceWords.size())) {
    index = &shader->storage_indices.source_words;
  } else if (AreStringsEqual(storage_descr.name, storage_descr.name_len,
                             kStorageNameDestinationWords.data(),
                             kStorageNameDestinationWords.size())) {
    index = &shader->storage_indices.destination_words;
  }

  COMET_ASSERT(index != nullptr, "Unable to find storage index with name \"",
               storage_descr.name, "\"!");
  COMET_ASSERT(*index == kInvalidShaderStorageIndex,
               "Storage index with name \"", storage_descr.name,
               "\" was already bound!");
  *index = static_cast<ShaderStorageIndex>(shader->storages.GetSize() - 1);
  return *index;
}

void ShaderHandler::HandleDescriptorSetLayoutsGeneration(Shader* shader) const {
  for (u32 i{0}; i < shader->layout_bindings.count; ++i) {
    const auto info{init::GenerateDescriptorSetLayoutCreateInfo(
        shader->layout_bindings.list[i])};
    COMET_CHECK_VK(
        vkCreateDescriptorSetLayout(context_->GetDevice(), &info,
                                    VK_NULL_HANDLE, &shader->layout_handles[i]),
        "Could not creat descriptor set layout for shader ",
        COMET_STRING_ID_LABEL(shader->id), "!");
  }
}

void ShaderHandler::HandlePipelineGeneration(Shader* shader) const {
  auto is_graphics{false};
  auto is_compute{false};

  for (const auto& module : shader->modules) {
    if (module->type == VK_SHADER_STAGE_COMPUTE_BIT) {
      is_compute = true;
    } else {
      is_graphics = true;
    }

    if (is_compute && is_graphics) {
      break;
    }
  }

  PipelineLayoutDescr layout_descr{};
  layout_descr.descriptor_set_layout_handles = &shader->layout_handles;
  layout_descr.descriptor_set_layout_count = shader->layout_bindings.count;
  layout_descr.push_constant_ranges = &shader->push_constant_ranges;

  auto* pipeline_layout{pipeline_handler_->GenerateLayout(layout_descr)};

  if (is_graphics) {
    HandleGraphicsPipelineGeneration(shader, pipeline_layout);
  }

  if (is_compute) {
    HandleComputePipelineGeneration(shader, pipeline_layout);
  }
}

void ShaderHandler::HandleGraphicsPipelineGeneration(
    Shader* shader, const PipelineLayout* pipeline_layout) const {
  const auto& device{context_->GetDevice()};

  GraphicsPipelineDescr pipeline_descr{};
  pipeline_descr.render_pass = shader->render_pass;
  pipeline_descr.layout_handle = pipeline_layout->handle;

  pipeline_descr.vertex_input_binding_description.binding = 0;
  pipeline_descr.vertex_input_binding_description.stride =
      shader->vertex_attribute_stride;
  pipeline_descr.vertex_input_binding_description.inputRate =
      VK_VERTEX_INPUT_RATE_VERTEX;
  pipeline_descr.vertex_attributes = &shader->vertex_attributes;

  pipeline_descr.shader_stages =
      frame::FrameArray<VkPipelineShaderStageCreateInfo>{};
  pipeline_descr.shader_stages.Reserve(shader->modules.GetSize());

  for (const auto& module : shader->modules) {
    if (module->type == VK_SHADER_STAGE_COMPUTE_BIT) {
      continue;
    }

    pipeline_descr.shader_stages.PushBack(
        init::GeneratePipelineShaderStageCreateInfo(module->type,
                                                    module->handle));
  }

  pipeline_descr.input_assembly_state =
      init::GeneratePipelineInputAssemblyStateCreateInfo(
          VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  pipeline_descr.rasterization_state =
      init::GeneratePipelineRasterizationStateCreateInfo(shader->is_wireframe,
                                                         shader->cull_mode);
  pipeline_descr.color_blend_attachment_state =
      init::GeneratePipelineColorBlendAttachmentState();
  pipeline_descr.multisample_state =
      init::GeneratePipelineMultisampleStateCreateInfo();

  if (device.IsMsaa()) {
    auto is_sample_rate_shading{context_->IsSampleRateShading()};

    pipeline_descr.multisample_state.rasterizationSamples =
        device.GetMsaaSamples();
    pipeline_descr.multisample_state.sampleShadingEnable =
        is_sample_rate_shading ? VK_TRUE : VK_FALSE;
    pipeline_descr.multisample_state.minSampleShading =
        is_sample_rate_shading ? .2f : .0f;  // Min fraction for sample shading;
                                             // closer to one is smoother.
  } else {
    pipeline_descr.multisample_state.rasterizationSamples =
        VK_SAMPLE_COUNT_1_BIT;
    pipeline_descr.multisample_state.sampleShadingEnable = VK_FALSE;
    pipeline_descr.multisample_state.minSampleShading = .0f;
  }

  pipeline_descr.depth_stencil_state =
      init::GeneratePipelineDepthStencilStateCreateInfo(
          true, true, VK_COMPARE_OP_LESS  // New fragments should be less
                                          // (lower depth = closer).
      );

  shader->graphics_pipeline = pipeline_handler_->Generate(pipeline_descr);
}

void ShaderHandler::HandleComputePipelineGeneration(
    Shader* shader, const PipelineLayout* pipeline_layout) const {
  ComputePipelineDescr pipeline_descr{};
  pipeline_descr.layout_handle = pipeline_layout->handle;

  for (const auto& module : shader->modules) {
    if (module->type != VK_SHADER_STAGE_COMPUTE_BIT) {
      continue;
    }

    if (pipeline_descr.shader_stage.stage == VK_SHADER_STAGE_COMPUTE_BIT) {
      COMET_LOG_RENDERING_ERROR(
          "Only one compute stage is allowed per shader! Ignoring current "
          "stage.");
      continue;
    }

    pipeline_descr.shader_stage = init::GeneratePipelineShaderStageCreateInfo(
        module->type, module->handle);
  }

  shader->compute_pipeline = pipeline_handler_->Generate(pipeline_descr);
}

void ShaderHandler::HandleDescriptorPoolGeneration(Shader* shader) const {
  constexpr VkDescriptorPoolSize pool_sizes[3]{
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4096},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1024}};

  shader->descriptor_pool_handle = shader->storage_data.descriptor_pool_handle =
      shader->global_uniform_data.descriptor_pool_handle =
          GenerateDescriptorPool(
              context_->GetDevice(), 1024, pool_sizes, 3,
              VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
}

void ShaderHandler::HandleUboBufferGeneration(Shader* shader) const {
  const auto align{context_->GetDevice()
                       .GetProperties()
                       .limits.minUniformBufferOffsetAlignment};

  shader->global_ubo_data.ubo_stride = memory::AlignSize(
      shader->global_ubo_data.ubo_size, static_cast<memory::Alignment>(align));
  shader->instance_ubo_data.ubo_stride =
      memory::AlignSize(shader->instance_ubo_data.ubo_size,
                        static_cast<memory::Alignment>(align));

#ifdef COMET_DEBUG
  const auto max_range{
      context_->GetDevice().GetProperties().limits.maxUniformBufferRange};
  COMET_ASSERT(shader->global_ubo_data.ubo_stride < max_range,
               "Shader global UBO data is too big!");
  COMET_ASSERT(shader->instance_ubo_data.ubo_stride < max_range,
               "Shader instance UBO data is too big!");
#endif  // COMET_DEBUG

  auto buffer_size{static_cast<VkDeviceSize>(
      shader->global_ubo_data.ubo_stride +
      shader->instance_ubo_data.ubo_stride * kMaxMaterialInstances)};

  if (buffer_size > 0) {
    shader->uniform_buffer = GenerateBuffer(
        context_->GetAllocatorHandle(), buffer_size,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0,
        VK_SHARING_MODE_EXCLUSIVE, "shader->uniform_buffer");
  }

  auto global_set_index{shader->descriptor_set_indices.global};

  if (global_set_index == kInvalidShaderDescriptorSetIndex) {
    return;
  }

  frame::FrameArray<VkDescriptorSetLayout> layout_buffer{};
  shader->global_uniform_data.descriptor_set_handles.Resize(
      descriptor_set_layout_count_);

  for (u32 i{0}; i < descriptor_set_layout_count_; ++i) {
    layout_buffer.PushBack(shader->layout_handles[global_set_index]);
  }

  auto is_allocated{descriptor_handler_->Generate(
      layout_buffer.GetData(),
      shader->global_uniform_data.descriptor_set_handles.GetData(),
      descriptor_set_layout_count_, DescriptorType::Static)};

  COMET_ASSERT(is_allocated,
               "Failed to allocate global uniform descriptor sets!");
}

void ShaderHandler::AddUniform(Shader* shader, const ShaderUniformDescr& descr,
                               ShaderUniformLocation location) const {
  COMET_ASSERT(shader->uniforms.GetSize() + 1 <= kMaxShaderUniformCount,
               "Too many uniforms to be added to shader ",
               COMET_STRING_ID_LABEL(shader->id), "! Max uniform count is ",
               kMaxShaderUniformCount, ".");
  ShaderUniform uniform{};
  uniform.scope = descr.scope;
  uniform.type = descr.type;
  uniform.index = HandleUniformIndex(shader, descr);
  uniform.stages = ResolveStageFlags(descr.stages);
  auto is_sampler{uniform.type == ShaderVariableType::Sampler};

  if (is_sampler) {
    uniform.location = location;
  } else {
    uniform.location = uniform.index;
  }

  auto size{ShaderVariableTypeSize(uniform.type)};

  uniform.offset =
      is_sampler ? 0
      : uniform.scope == ShaderUniformScope::Global
          ? static_cast<ShaderOffset>(shader->global_ubo_data.ubo_size)
          : static_cast<ShaderOffset>(shader->instance_ubo_data.ubo_size);
  uniform.size = size;

  shader->uniforms.PushBack(uniform);

  if (uniform.scope == ShaderUniformScope::Global) {
    shader->global_ubo_data.stages |= uniform.stages;
  } else if (uniform.scope == ShaderUniformScope::Instance) {
    shader->instance_ubo_data.stages |= uniform.stages;
  }

  if (!is_sampler) {
    if (uniform.scope == ShaderUniformScope::Global) {
      shader->global_ubo_data.ubo_size += uniform.size;
    } else if (uniform.scope == ShaderUniformScope::Instance) {
      shader->instance_ubo_data.ubo_size += uniform.size;
    }
  }
}

void ShaderHandler::AddConstant(Shader* shader,
                                const ShaderConstantDescr& descr) const {
  COMET_ASSERT(shader->constants.GetSize() + 1 <= kMaxShaderUniformCount,
               "Too many constants to be added to shader ",
               COMET_STRING_ID_LABEL(shader->id), "! Max constant count is ",
               kMaxShaderConstantCount, ".");
  ShaderConstant constant{};
  constant.type = descr.type;
  constant.index = HandleConstantIndex(shader, descr);
  constant.stages = ResolveStageFlags(descr.stages);
  constant.offset =
      static_cast<ShaderOffset>(shader->push_constant_ranges.GetSize());

  auto size{ShaderVariableTypeSize(constant.type)};

  if (shader->constants.IsEmpty()) {
    constant.offset = 0;
  } else {
    const auto& last_constant{shader->constants.GetLast()};
    constant.offset = last_constant.offset + last_constant.size;
  }

  constant.size = static_cast<ShaderUniformSize>(memory::AlignSize(
      size, static_cast<memory::Alignment>(GetStd430Alignment(constant.type))));

  VkPushConstantRange range{};
  range.offset = constant.offset;
  range.size = constant.size;
  range.stageFlags = constant.stages;

  shader->constants.PushBack(constant);
  shader->push_constant_ranges.PushBack(range);
}

void ShaderHandler::AddStorage(Shader* shader,
                               const ShaderStorageDescr& descr) const {
  auto& storage{shader->storages.EmplaceBack()};
  storage.index = HandleStorageIndex(shader, descr);
  storage.stages = ResolveStageFlags(descr.stages);
  shader->storage_data.stages |= storage.stages;

  auto property_count{
      static_cast<ShaderStoragePropertySize>(descr.properties.GetSize())};

  if (property_count > kMaxShaderStorageLayoutPropertyCount) {
    COMET_LOG_RENDERING_ERROR("Too many storage properties: ", property_count,
                              "! Max is ", kMaxShaderStorageLayoutPropertyCount,
                              ". Exceeding properties will be ignored.");
    property_count = kMaxShaderStorageLayoutPropertyCount;
  }

  storage.property_count = property_count;

  for (ShaderStoragePropertySize i{0}; i < storage.property_count; ++i) {
    const auto& property_descr{descr.properties[i]};
    auto& property{storage.properties[i]};
    property.type = property_descr.type;
    property.size = ShaderVariableTypeSize(property.type);
  }
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet
