// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_shader_handler.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <algorithm>
#include <type_traits>
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_string.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/core/memory/allocator/allocator.h"
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
#include "comet/rendering/driver/vulkan/utils/vulkan_shader_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/resource_manager.h"

namespace comet {
namespace rendering {
namespace vk {
namespace internal {
constexpr u32 kGlobalSet{0};
constexpr u32 kMaterialSet{1};
constexpr u32 kPassSet{2};

bool HasBindingInSet(const Shader* shader, u32 set) {
  for (const auto& binding : shader->bindings) {
    if (binding.set == set) {
      return true;
    }
  }
  return false;
}
}  // namespace internal

ShaderHandler::ShaderHandler(const ShaderHandlerDescr& descr)
    : Handler{descr},
      shader_module_handler_{descr.shader_module_handler},
      pipeline_handler_{descr.pipeline_handler},
      material_handler_{descr.material_handler},
      texture_handler_{descr.texture_handler},
      descriptor_handler_{descr.descriptor_handler},
      render_pass_handler_{descr.render_pass_handler} {
  COMET_ASSERT(shader_module_handler_ != nullptr,
               "Shader module handler is null!");
  COMET_ASSERT(pipeline_handler_ != nullptr, "Pipeline handler is null!");
  COMET_ASSERT(material_handler_ != nullptr, "Material handler is null!");
  COMET_ASSERT(texture_handler_ != nullptr, "Texture handler is null!");
  COMET_ASSERT(descriptor_handler_ != nullptr, "Descriptor handler is null!");
  COMET_ASSERT(render_pass_handler_ != nullptr, "Render pass handler is null!");
}

void ShaderHandler::Initialize() {
  Handler::Initialize();
  general_allocator_.Initialize();
  shader_instance_allocator_.Initialize();
  shaders_ = Array<Shader*>{&shader_instance_allocator_};
  shader_keys_ =
      Map<ShaderKey, ShaderHandle, ShaderKeyHashLogic>{&general_allocator_};
  descriptor_set_layout_count_ = context_->GetImageCount();

  material_handler_->SetDestroyCallback(
      [](Material* material, void* user_data) {
        auto* shader_handler{static_cast<ShaderHandler*>(user_data)};
        shader_handler->UnbindMaterialFromAllShaders(material);
      },
      this);
}

void ShaderHandler::Shutdown() {
  material_handler_->SetDestroyCallback(nullptr, nullptr);
  descriptor_set_layout_count_ = 0;

  for (auto* shader : shaders_) {
    if (shader != nullptr) {
      Destroy(shader, true);
    }
  }

  shader_keys_.Destroy();
  shaders_.Destroy();
  bound_shader_ = nullptr;
  shader_instance_allocator_.Destroy();
  general_allocator_.Destroy();
  Handler::Shutdown();
}

Shader* ShaderHandler::Get(ShaderHandle shader_handle) {
  COMET_ASSERT(shader_handle_handler_.IsAlive(shader_handle));
  auto* shader{TryGet(shader_handle)};
  COMET_ASSERT(shader != nullptr, "Requested shader does not exist: ",
               COMET_STRING_ID_LABEL(shader_handle), "!");
  return shader;
}

const Shader* ShaderHandler::Get(ShaderHandle shader_handle) const {
  COMET_ASSERT(shader_handle_handler_.IsAlive(shader_handle));
  auto* shader{TryGet(shader_handle)};
  COMET_ASSERT(shader != nullptr, "Requested shader does not exist: ",
               COMET_STRING_ID_LABEL(shader_handle), "!");
  return shader;
}

Shader* ShaderHandler::TryGet(ShaderHandle shader_handle) {
  if (!shader_handle_handler_.IsAlive(shader_handle)) return nullptr;
  return shaders_[gid::GetIndex(shader_handle)];
}

const Shader* ShaderHandler::TryGet(ShaderHandle shader_handle) const {
  if (!shader_handle_handler_.IsAlive(shader_handle)) return nullptr;
  return shaders_[gid::GetIndex(shader_handle)];
}

Shader* ShaderHandler::GetOrGenerate(const ShaderDescr& descr) {
  return GetOrGenerate(descr.shader_id, descr.render_pass_handle);
}

Shader* ShaderHandler::GetOrGenerate(resource::ResourceId shader_id,
                                     RenderPassHandle render_pass_handle) {
  COMET_ASSERT(shader_id != resource::kInvalidResourceId,
               "Shader resource id is invalid!");
  COMET_ASSERT(render_pass_handle != kInvalidRenderPassHandle,
               "Render pass handle is invalid!");

  ShaderKey key{};
  key.shader_id = shader_id;
  key.render_pass_handle = render_pass_handle;

  if (auto* shader_handle{shader_keys_.TryGet(key)}; shader_handle != nullptr) {
    auto* shader{Get(*shader_handle)};
    ++shader->ref_count;
    return shader;
  }

  const auto* shader_resource{
      resource::ResourceManager::Get().GetShaders()->Load(shader_id)};
  COMET_ASSERT(shader_resource != nullptr, "Shader resource is null!");

  ShaderDescr descr{};
  descr.shader_id = shader_id;
  descr.render_pass_handle = render_pass_handle;

  auto* shader{Generate(descr, shader_resource)};
  shader_keys_.Emplace(key, shader->handle);
  return shader;
}

void ShaderHandler::UnbindMaterialFromAllShaders(Material* material) {
  COMET_ASSERT(material != nullptr, "Material is null!");

  for (auto* shader : shaders_) {
    if (shader == nullptr) {
      continue;
    }

    if (!HasMaterial(shader, material)) {
      continue;
    }

    UnbindMaterial(shader, material);
  }
}

void ShaderHandler::Destroy(ShaderHandle shader_handle) {
  Destroy(Get(shader_handle));
}

void ShaderHandler::Destroy(Shader* shader) { Destroy(shader, false); }

ShaderBindingIndex ShaderHandler::GetBindingIndex(const Shader* shader, u32 set,
                                                  u32 binding) const {
  COMET_ASSERT(shader != nullptr, "Shader is null!");

  for (ShaderBindingIndex i{0}; i < shader->bindings.GetSize(); ++i) {
    const auto& shader_binding{shader->bindings[i]};

    if (shader_binding.set == set && shader_binding.binding == binding) {
      return i;
    }
  }

  COMET_ASSERT(false, "Unable to find shader binding in set ", set,
               ", binding ", binding, "!");
  return 0;
}

void ShaderHandler::Bind(const Shader* shader,
                         PipelineBindType pipeline_type) const {
  const Pipeline* pipeline{nullptr};

  if (pipeline_type == PipelineBindType::Graphics) {
    pipeline = shader->graphics_pipeline;
  } else if (pipeline_type == PipelineBindType::Compute) {
    pipeline = shader->compute_pipeline;
  }

  COMET_ASSERT(pipeline != nullptr, "Tried to bind an invalid pipeline!");
  pipeline_handler_->Bind(pipeline);

  auto bind_point{pipeline->type == PipelineBindType::Graphics
                      ? VK_PIPELINE_BIND_POINT_GRAPHICS
                      : VK_PIPELINE_BIND_POINT_COMPUTE};

  auto command_buffer_handle{context_->GetFrameData().command_buffer_handle};
  auto image_index{context_->GetImageIndex()};

  if (!shader->global_descriptor_data.descriptor_set_handles.IsEmpty() &&
      internal::HasBindingInSet(shader, internal::kGlobalSet)) {
    auto set_handle{
        shader->global_descriptor_data.descriptor_set_handles[image_index]};

    vkCmdBindDescriptorSets(command_buffer_handle, bind_point,
                            pipeline->layout_handle, internal::kGlobalSet, 1,
                            &set_handle, 0, VK_NULL_HANDLE);
  }

  if (!shader->storage_descriptor_data.descriptor_set_handles.IsEmpty() &&
      internal::HasBindingInSet(shader, internal::kPassSet)) {
    auto set_handle{
        shader->storage_descriptor_data.descriptor_set_handles[image_index]};

    vkCmdBindDescriptorSets(command_buffer_handle, bind_point,
                            pipeline->layout_handle, internal::kPassSet, 1,
                            &set_handle, 0, VK_NULL_HANDLE);
  }
}

void ShaderHandler::BindInstance(Shader* shader, MaterialId material_id,
                                 PipelineBindType pipeline_type) {
  BindInstance(shader, material_handler_->Get(material_id), pipeline_type);
}

void ShaderHandler::BindInstance(Shader* shader, const Material* material,
                                 PipelineBindType pipeline_type) {
  if (!internal::HasBindingInSet(shader, internal::kMaterialSet)) {
    return;
  }

  const Pipeline* pipeline{nullptr};

  if (pipeline_type == PipelineBindType::Graphics) {
    pipeline = shader->graphics_pipeline;
  } else if (pipeline_type == PipelineBindType::Compute) {
    pipeline = shader->compute_pipeline;
  }

  COMET_ASSERT(pipeline != nullptr, "Tried to bind an invalid pipeline!");
  pipeline_handler_->Bind(pipeline);

  auto bind_point{pipeline->type == PipelineBindType::Graphics
                      ? VK_PIPELINE_BIND_POINT_GRAPHICS
                      : VK_PIPELINE_BIND_POINT_COMPUTE};

  auto command_buffer_handle{context_->GetFrameData().command_buffer_handle};
  auto image_index{context_->GetImageIndex()};
  auto& instance{GetInstance(shader, material)};

  if (instance.descriptor_data.descriptor_set_handles.IsEmpty()) {
    return;
  }

  auto set_handle{instance.descriptor_data.descriptor_set_handles[image_index]};

  vkCmdBindDescriptorSets(command_buffer_handle, bind_point,
                          pipeline->layout_handle, internal::kMaterialSet, 1,
                          &set_handle, 0, VK_NULL_HANDLE);
}

void ShaderHandler::Reset() { bound_shader_ = nullptr; }

void ShaderHandler::UpdatePass(Shader* shader,
                               const ShaderPassUpdate& update) const {
  COMET_ASSERT(shader != nullptr, "Shader is null!");

  auto image_index{context_->GetImageIndex()};

  if (shader->storage_descriptor_data.descriptor_set_handles.IsEmpty()) {
    return;
  }

  auto set_handle{
      shader->storage_descriptor_data.descriptor_set_handles[image_index]};

  if (update.buffer_bindings != nullptr && !update.buffer_bindings->IsEmpty()) {
    for (const auto& buffer_update : *update.buffer_bindings) {
      COMET_ASSERT(buffer_update.binding_index < shader->bindings.GetSize(),
                   "Invalid binding index in pass buffer update!");

      const auto& binding{shader->bindings[buffer_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Pass,
                   "Pass buffer update targets non-pass binding!");
      COMET_ASSERT(binding.type == ShaderBindingType::UniformBuffer ||
                       binding.type == ShaderBindingType::StorageBuffer,
                   "Pass buffer update targets non-buffer binding!");

      UpdateDescriptorSetBuffer(
          set_handle, binding, buffer_update.buffer_handle,
          buffer_update.buffer_size, buffer_update.buffer_offset);
    }
  }

  if (update.image_bindings != nullptr && !update.image_bindings->IsEmpty()) {
    for (const auto& image_update : *update.image_bindings) {
      COMET_ASSERT(image_update.binding_index < shader->bindings.GetSize(),
                   "Invalid binding index in pass image update!");

      const auto& binding{shader->bindings[image_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Pass,
                   "Pass image update targets non-pass binding!");
      COMET_ASSERT(IsImageBindingType(binding.type),
                   "Pass image update targets non-image binding!");
      COMET_ASSERT(image_update.descriptors != nullptr,
                   "Pass image update descriptors are null!");
      COMET_ASSERT(image_update.descriptor_count > 0,
                   "Pass image update descriptor count is 0!");

      UpdateDescriptorSetImages(set_handle, binding, image_update.descriptors,
                                image_update.descriptor_count);
    }
  }
}

void ShaderHandler::UpdateGlobals(Shader* shader,
                                  const ShaderGlobalUpdate& update) const {
  COMET_ASSERT(shader != nullptr, "Shader is null!");

  auto image_index{context_->GetImageIndex()};

  if (shader->global_descriptor_data.descriptor_set_handles.IsEmpty()) {
    return;
  }

  auto set_handle{
      shader->global_descriptor_data.descriptor_set_handles[image_index]};

  if (update.field_updates != nullptr && !update.field_updates->IsEmpty()) {
    auto& buffer{shader->uniform_buffers[image_index]};
    ScopedMappedBuffer mapped{buffer};

    for (const auto& field_update : *update.field_updates) {
      COMET_ASSERT(field_update.binding_index < shader->bindings.GetSize(),
                   "Invalid binding index in global field update!");

      const auto& binding{shader->bindings[field_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Global,
                   "Global field update targets non-global binding!");
      COMET_ASSERT(binding.type == ShaderBindingType::UniformBuffer,
                   "Global field update targets non-UBO binding!");

      WriteBindingFieldToMappedUbo(buffer, shader->bound_global_ubo_offset,
                                   binding, field_update.field_index,
                                   field_update.data, field_update.size);
    }
  }

  if (update.buffer_bindings != nullptr && !update.buffer_bindings->IsEmpty()) {
    for (const auto& buffer_update : *update.buffer_bindings) {
      COMET_ASSERT(buffer_update.binding_index < shader->bindings.GetSize(),
                   "Invalid binding index in global buffer update!");

      const auto& binding{shader->bindings[buffer_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Global,
                   "Global buffer update targets non-global binding!");
      COMET_ASSERT(binding.type == ShaderBindingType::UniformBuffer ||
                       binding.type == ShaderBindingType::StorageBuffer,
                   "Global buffer update targets non-buffer binding!");

      UpdateDescriptorSetBuffer(
          set_handle, binding, buffer_update.buffer_handle,
          buffer_update.buffer_size, buffer_update.buffer_offset);
    }
  }

  if (update.image_bindings != nullptr && !update.image_bindings->IsEmpty()) {
    for (const auto& image_update : *update.image_bindings) {
      COMET_ASSERT(image_update.binding_index < shader->bindings.GetSize(),
                   "Invalid binding index in global image update!");

      const auto& binding{shader->bindings[image_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Global,
                   "Global image update targets non-global binding!");
      COMET_ASSERT(IsImageBindingType(binding.type),
                   "Global image update targets non-image binding!");
      COMET_ASSERT(image_update.descriptors != nullptr,
                   "Global image update descriptors are null!");
      COMET_ASSERT(image_update.descriptor_count > 0,
                   "Global image update descriptor count is 0!");

      UpdateDescriptorSetImages(set_handle, binding, image_update.descriptors,
                                image_update.descriptor_count);
    }
  }
}

void ShaderHandler::UpdateInstance(Shader* shader, MaterialId material_id,
                                   const ShaderInstanceUpdate& update) {
  UpdateInstance(shader, material_handler_->Get(material_id), update);
}

void ShaderHandler::UpdateInstance(Shader* shader, Material* material,
                                   const ShaderInstanceUpdate& update) {
  COMET_ASSERT(shader != nullptr, "Shader is null!");
  COMET_ASSERT(material != nullptr, "Material is null!");

  auto& instance{GetInstance(shader, material)};
  auto image_index{context_->GetImageIndex()};

  if (instance.descriptor_data.descriptor_set_handles.IsEmpty()) {
    return;
  }

  auto set_handle{instance.descriptor_data.descriptor_set_handles[image_index]};

  shader->bound_instance_ubo_offset =
      shader->global_ubo_data.stride + instance.offset;

  if (update.field_updates != nullptr && !update.field_updates->IsEmpty()) {
    auto& buffer{shader->uniform_buffers[image_index]};
    ScopedMappedBuffer mapped{buffer};

    for (const auto& field_update : *update.field_updates) {
      COMET_ASSERT(field_update.binding_index < shader->bindings.GetSize(),
                   "Invalid binding index in instance field update!");

      const auto& binding{shader->bindings[field_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Material,
                   "Instance field update targets non-material binding!");
      COMET_ASSERT(binding.type == ShaderBindingType::UniformBuffer,
                   "Instance field update targets non-UBO binding!");

      WriteBindingFieldToMappedUbo(buffer, shader->bound_instance_ubo_offset,
                                   binding, field_update.field_index,
                                   field_update.data, field_update.size);
    }
  }

  if (update.buffer_bindings != nullptr && !update.buffer_bindings->IsEmpty()) {
    for (const auto& buffer_update : *update.buffer_bindings) {
      COMET_ASSERT(buffer_update.binding_index < shader->bindings.GetSize(),
                   "Invalid binding index in instance buffer update!");

      const auto& binding{shader->bindings[buffer_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Material,
                   "Instance buffer update targets non-material binding!");
      COMET_ASSERT(binding.type == ShaderBindingType::UniformBuffer ||
                       binding.type == ShaderBindingType::StorageBuffer,
                   "Instance buffer update targets non-buffer binding!");

      UpdateDescriptorSetBuffer(
          set_handle, binding, buffer_update.buffer_handle,
          buffer_update.buffer_size, buffer_update.buffer_offset);
    }
  }

  if (update.image_bindings != nullptr && !update.image_bindings->IsEmpty()) {
    for (const auto& image_update : *update.image_bindings) {
      COMET_ASSERT(image_update.binding_index < shader->bindings.GetSize(),
                   "Invalid binding index in instance image update!");

      const auto& binding{shader->bindings[image_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Material,
                   "Instance image update targets non-material binding!");
      COMET_ASSERT(IsImageBindingType(binding.type),
                   "Instance image update targets non-image binding!");
      COMET_ASSERT(image_update.descriptors != nullptr,
                   "Instance image update descriptors are null!");
      COMET_ASSERT(image_update.descriptor_count > 0,
                   "Instance image update descriptor count is 0!");

      UpdateDescriptorSetImages(set_handle, binding, image_update.descriptors,
                                image_update.descriptor_count);

      if (image_index == 0) {
        auto* runtime_binding{FindBindingRuntimeData(
            instance.binding_data, image_update.binding_index)};

        if (runtime_binding == nullptr) {
          ShaderBindingImageRuntimeData new_runtime_binding{};
          new_runtime_binding.binding_index = image_update.binding_index;
          new_runtime_binding.descriptors =
              Array<ShaderImageDescriptor>{&general_allocator_};
          new_runtime_binding.descriptors.PushFromRange(
              image_update.descriptors, image_update.descriptor_count);
          instance.binding_data.image_bindings.EmplaceBack(
              std::move(new_runtime_binding));
        } else {
          runtime_binding->descriptors.Clear();
          runtime_binding->descriptors.PushFromRange(
              image_update.descriptors, image_update.descriptor_count);
        }
      }
    }
  }
}

void ShaderHandler::PushConstants(
    Shader* shader, const ShaderPushConstantsUpdate& update) const {
  if (update.blocks == nullptr) {
    return;
  }

  auto command_buffer_handle{context_->GetFrameData().command_buffer_handle};

  for (const auto& block_update : *update.blocks) {
    COMET_ASSERT(
        block_update.block_index < shader->push_constant_blocks.GetSize(),
        "Invalid push constant block index!");

    const auto& block{shader->push_constant_blocks[block_update.block_index]};
    COMET_ASSERT(block_update.data != nullptr,
                 "Push constant block data is null!");
    COMET_ASSERT(block_update.size <= static_cast<u32>(block.size),
                 "Push constant update exceeds block size!");

    if (shader->graphics_pipeline != nullptr && IsGraphicsStage(block.stages)) {
      vkCmdPushConstants(
          command_buffer_handle, shader->graphics_pipeline->layout_handle,
          block.stages, static_cast<u32>(block.offset),
          static_cast<u32>(block_update.size), block_update.data);
    }

    if (shader->compute_pipeline != nullptr && IsComputeStage(block.stages)) {
      vkCmdPushConstants(
          command_buffer_handle, shader->compute_pipeline->layout_handle,
          block.stages, static_cast<u32>(block.offset),
          static_cast<u32>(block_update.size), block_update.data);
    }
  }
}

void ShaderHandler::BindMaterial(Shader* shader, Material* material) {
  COMET_ASSERT(shader != nullptr, "Shader is null!");
  COMET_ASSERT(material != nullptr, "Material is null!");
  COMET_ASSERT(!HasMaterial(shader, material),
               "Material is already bound to this shader instance!");
  COMET_ASSERT(shader->instances.list.GetSize() < kMaxMaterialInstances,
               "Maximum material instance count reached!");

  if (!internal::HasBindingInSet(shader, internal::kMaterialSet)) {
    return;
  }

  MaterialInstance instance{};
  instance.descriptor_data.descriptor_set_handles =
      Array<VkDescriptorSet>{&general_allocator_};
  instance.binding_data.image_bindings =
      Array<ShaderBindingImageRuntimeData>{&general_allocator_};

  u32 material_image_binding_count{0};

  for (const auto& binding : shader->bindings) {
    if (binding.set == internal::kMaterialSet &&
        IsImageBindingType(binding.type)) {
      ++material_image_binding_count;
    }
  }

  instance.binding_data.image_bindings.Reserve(material_image_binding_count);
  auto instance_index{static_cast<u32>(shader->instances.list.GetSize())};
  AllocateMaterialDescriptorSets(shader, instance);
  instance.material_id = material->id;
  UpdateInstanceUboDescriptors(shader, instance, instance_index);

  auto& resolved_image_descriptors{*COMET_FRAME_ARRAY(ShaderImageDescriptor)};

  for (u32 image_index{0}; image_index < descriptor_set_layout_count_;
       ++image_index) {
    auto set_handle{
        instance.descriptor_data.descriptor_set_handles[image_index]};

    for (const auto& binding : shader->bindings) {
      if (binding.set != internal::kMaterialSet) {
        continue;
      }

      if (IsImageBindingType(binding.type)) {
        CollectMaterialImageDescriptors(material, binding,
                                        resolved_image_descriptors);

        if (image_index == 0) {
          ShaderBindingImageRuntimeData runtime_binding{};
          runtime_binding.binding_index = binding.index;
          runtime_binding.descriptors =
              Array<ShaderImageDescriptor>{&general_allocator_};
          runtime_binding.descriptors.PushFromRange(resolved_image_descriptors);
          instance.binding_data.image_bindings.EmplaceBack(
              std::move(runtime_binding));
        }

        UpdateDescriptorSetImages(
            set_handle, binding, resolved_image_descriptors.GetData(),
            static_cast<u32>(resolved_image_descriptors.GetSize()));
      }
    }
  }

  shader->instances.indices.Emplace(material->id, instance_index);
  shader->instances.list.EmplaceBack(std::move(instance));
}

void ShaderHandler::UnbindMaterial(Shader* shader, Material* material) {
  COMET_ASSERT(shader != nullptr, "Shader is null!");
  COMET_ASSERT(material != nullptr, "Material is null!");
  COMET_ASSERT(HasMaterial(shader, material),
               "Trying to destroy dead material instance for material ",
               material->id, "!");

  auto* index_ptr{shader->instances.indices.TryGet(material->id)};
  COMET_ASSERT(index_ptr != nullptr, "Material instance index is null!");

  const u32 index{*index_ptr};
  auto& instances{shader->instances.list};
  auto& instance{instances[index]};
  auto& device{context_->GetDevice()};

  auto& descriptor_data{instance.descriptor_data};
  FreeDescriptor(device, descriptor_data.descriptor_set_handles,
                 descriptor_data.descriptor_pool_handle);

  DestroyBindingRuntimeData(instance.binding_data);

  const u32 last_index{static_cast<u32>(instances.GetSize() - 1)};
  const MaterialId removed_material_id{material->id};

  if (index != last_index) {
    instances[index] = std::move(instances[last_index]);
    auto& moved_instance{instances[index]};
    shader->instances.indices[moved_instance.material_id] = index;
    UpdateInstanceUboDescriptors(shader, moved_instance, index);
  }

  instances.Resize(last_index);
  shader->instances.indices.Remove(removed_material_id);
}

bool ShaderHandler::HasMaterial(const Shader* shader,
                                const Material* material) const {
  COMET_ASSERT(shader != nullptr, "Shader is null!");
  COMET_ASSERT(material != nullptr, "Material is null!");
  return shader->instances.indices.TryGet(material->id) != nullptr;
}

MaterialInstance& ShaderHandler::GetInstance(Shader* shader,
                                             const Material* material) {
  COMET_ASSERT(shader != nullptr, "Shader is null!");
  COMET_ASSERT(material != nullptr, "Material is null!");

  auto* index{shader->instances.indices.TryGet(material->id)};
  COMET_ASSERT(index != nullptr, "Material ", material->id,
               " is not bound to shader ",
               COMET_STRING_ID_LABEL(shader->handle), "!");

  return shader->instances.list[*index];
}

const MaterialInstance* ShaderHandler::TryGetInstance(
    const Shader* shader, const Material* material) const {
  if (shader == nullptr || material == nullptr) {
    return nullptr;
  }

  auto* index{shader->instances.indices.TryGet(material->id)};
  if (index == nullptr) {
    return nullptr;
  }

  return &shader->instances.list[*index];
}

bool ShaderHandler::HasVertexStage(const Shader* shader) {
  for (const auto& module : shader->modules) {
    if (module->type == VK_SHADER_STAGE_VERTEX_BIT) {
      return true;
    }
  }

  return false;
}

VkDescriptorType ShaderHandler::GetVkDescriptorType(ShaderBindingType type) {
  switch (type) {
    case ShaderBindingType::UniformBuffer:
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case ShaderBindingType::StorageBuffer:
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case ShaderBindingType::CombinedImageSampler:
      return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case ShaderBindingType::SampledImage:
      return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case ShaderBindingType::Sampler:
      return VK_DESCRIPTOR_TYPE_SAMPLER;
    case ShaderBindingType::StorageImage:
      return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    default:
      COMET_ASSERT(false, "Unsupported shader binding type!");
  }

  return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

Alignment ShaderHandler::GetBindingFieldAlignment(ShaderMemoryLayout layout,
                                                  ShaderVariableType type) {
  switch (layout) {
    case ShaderMemoryLayout::Std140:
      return GetStd140Alignment(type);
    case ShaderMemoryLayout::Std430:
      return GetStd430Alignment(type);
    case ShaderMemoryLayout::Packed:
      return GetScalarAlignment(type);
    default:
      COMET_ASSERT(false, "Unknown shader memory layout!");
  }

  return kInvalidAlignment;
}

ShaderOffset ShaderHandler::AlignOffset(ShaderOffset offset,
                                        Alignment alignment) {
  return static_cast<ShaderOffset>(
      memory::AlignSize(offset, static_cast<memory::Alignment>(alignment)));
}

ShaderFieldLayoutInfo ShaderHandler::GetFieldLayoutInfo(
    ShaderMemoryLayout layout, ShaderVariableType type, u32 array_count) {
  ShaderFieldLayoutInfo info{};

  info.element_size = GetShaderVariableTypeSize(type);
  COMET_ASSERT(info.element_size != kInvalidShaderVariableSize,
               "Invalid shader variable type size!");

  info.alignment = GetBindingFieldAlignment(layout, type);

  info.aligned_size = static_cast<ShaderVariableSize>(memory::AlignSize(
      info.element_size, static_cast<memory::Alignment>(info.alignment)));

  info.stride = info.aligned_size;

  if (array_count > 1) {
    info.total_size =
        static_cast<ShaderVariableSize>(info.stride * array_count);
  } else {
    info.total_size = info.aligned_size;
  }

  return info;
}

void ShaderHandler::PopulateVertexAttributes(
    ShaderVertexLayout layout,
    Array<VkVertexInputAttributeDescription>& attributes, u32& stride) {
  switch (layout) {
    case ShaderVertexLayout::None:
      attributes.Clear();
      stride = 0;
      return;

    case ShaderVertexLayout::SkinnedVertex:
      PopulateSkinnedVertexAttributes(attributes, stride);
      return;

    case ShaderVertexLayout::DebugLine:
      PopulateDebugLineVertexAttributes(attributes, stride);
      return;

    default:
      COMET_ASSERT(
          false, "Unsupported shader vertex layout: ",
          static_cast<std::underlying_type_t<ShaderVertexLayout>>(layout), "!");
      attributes.Clear();
      stride = 0;
      return;
  }
}

void ShaderHandler::PopulateSkinnedVertexAttributes(
    Array<VkVertexInputAttributeDescription>& attributes, u32& stride) {
  attributes.Clear();
  attributes.Reserve(7);

  attributes.PushBack(VkVertexInputAttributeDescription{
      .location = 0,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = static_cast<u32>(offsetof(geometry::SkinnedVertex, position)),
  });

  attributes.PushBack(VkVertexInputAttributeDescription{
      .location = 1,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = static_cast<u32>(offsetof(geometry::SkinnedVertex, normal)),
  });

  attributes.PushBack(VkVertexInputAttributeDescription{
      .location = 2,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32A32_SFLOAT,
      .offset = static_cast<u32>(offsetof(geometry::SkinnedVertex, tangent)),
  });

  attributes.PushBack(VkVertexInputAttributeDescription{
      .location = 3,
      .binding = 0,
      .format = VK_FORMAT_R32G32_SFLOAT,
      .offset = static_cast<u32>(offsetof(geometry::SkinnedVertex, uv)),
  });

  attributes.PushBack(VkVertexInputAttributeDescription{
      .location = 4,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32A32_SFLOAT,
      .offset = static_cast<u32>(offsetof(geometry::SkinnedVertex, color)),
  });

  attributes.PushBack(VkVertexInputAttributeDescription{
      .location = 5,
      .binding = 0,
      .format = VK_FORMAT_R16G16B16A16_UINT,
      .offset =
          static_cast<u32>(offsetof(geometry::SkinnedVertex, joint_indices)),
  });

  attributes.PushBack(VkVertexInputAttributeDescription{
      .location = 6,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32A32_SFLOAT,
      .offset =
          static_cast<u32>(offsetof(geometry::SkinnedVertex, joint_weights)),
  });

  stride = static_cast<u32>(sizeof(geometry::SkinnedVertex));
}

void ShaderHandler::PopulateDebugLineVertexAttributes(
    Array<VkVertexInputAttributeDescription>& attributes, u32& stride) {
  attributes.Clear();
  attributes.Reserve(1);

  attributes.PushBack(VkVertexInputAttributeDescription{
      .location = 0,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32A32_SFLOAT,
      .offset = 0,
  });

  stride = sizeof(math::Vec4);
}

Shader* ShaderHandler::Generate(
    const ShaderDescr& descr, const resource::ShaderResource* shader_resource) {
  COMET_ASSERT(shader_resource != nullptr, "Shader resource is null!");
  auto* shader{shader_instance_allocator_.AllocateOneAndPopulate<Shader>()};
  auto shader_handle{shader_handle_handler_.Generate()};

  shader->handle = shader_handle;

  shader->id = shader_resource->id;
  shader->render_pass_handle = descr.render_pass_handle;
  shader->ref_count = 1;

  const auto& rasterizer_resource{shader_resource->descr.rasterizer};

  shader->rasterizer = {
      .is_wireframe = rasterizer_resource.is_wireframe,
      .is_depth_bias = rasterizer_resource.is_depth_bias,
      .cull_mode = rasterizer_resource.cull_mode,
  };

  const auto& depth_stencil_resource{shader_resource->descr.depth_stencil};

  shader->depth_stencil = {
      .is_depth_test = depth_stencil_resource.is_depth_test,
      .is_depth_write = depth_stencil_resource.is_depth_write,
      .compare_op = depth_stencil_resource.compare_op,
  };

  shader->topology = shader_resource->descr.topology;
  shader->vertex_layout = shader_resource->descr.vertex_layout;

  shader->vertex_attributes =
      Array<VkVertexInputAttributeDescription>{&general_allocator_};
  shader->bindings = Array<ShaderBinding>{&general_allocator_};
  shader->push_constant_blocks =
      Array<ShaderPushConstantBlock>{&general_allocator_};
  shader->modules = Array<const ShaderModule*>{&general_allocator_};
  shader->push_constant_ranges =
      Array<VkPushConstantRange>{&general_allocator_};

  shader->global_descriptor_data.descriptor_set_handles =
      Array<VkDescriptorSet>{&general_allocator_};

  shader->storage_descriptor_data.descriptor_set_handles =
      Array<VkDescriptorSet>{&general_allocator_};

  shader->instances.list = Array<MaterialInstance>{&shader_instance_allocator_};
  shader->instances.indices = Map<MaterialId, u32>{&general_allocator_};

  HandleShaderModulesGeneration(shader, shader_resource);
  HandleAttributesGeneration(shader, shader_resource);
  HandleBindingsGeneration(shader, shader_resource);
  HandlePushConstantBlocksGeneration(shader, shader_resource);
  HandleDescriptorSetLayoutsGeneration(shader);
  HandlePipelineGeneration(shader);
  HandleDescriptorPoolGeneration(shader);
  HandleUboBufferGeneration(shader);

  AllocateGlobalDescriptorSets(shader);
  AllocateStorageDescriptorSets(shader);

  auto shader_index{gid::GetIndex(shader_handle)};
  shaders_.Resize(shader_index + 1);
  shaders_[shader_index] = shader;
  return shader;
}

void ShaderHandler::Destroy(Shader* shader, bool is_destroying_handler) {
  COMET_ASSERT(shader != nullptr, "Shader is null!");
  auto shader_index{gid::GetIndex(shader->handle)};

  if (!is_destroying_handler) {
    COMET_ASSERT(shader->ref_count > 0, "Shader ref count is 0!");

    if (--shader->ref_count > 0) {
      return;
    }

    ShaderKey key{};
    key.shader_id = shader->id;
    key.render_pass_handle = shader->render_pass_handle;
    shader_keys_.Remove(key);
    shader_handle_handler_.Destroy(shader->handle);
  }

  if (shader_index < shaders_.GetSize()) {
    shaders_[shader_index] = nullptr;
  }

  const auto& device{context_->GetDevice()};

  shader->global_descriptor_data.descriptor_set_handles.Destroy();
  shader->storage_descriptor_data.descriptor_set_handles.Destroy();

  shader->global_descriptor_data.descriptor_pool_handle = VK_NULL_HANDLE;
  shader->storage_descriptor_data.descriptor_pool_handle = VK_NULL_HANDLE;
  shader->global_descriptor_data.update_frame = kInvalidFrameIndex;
  shader->storage_descriptor_data.update_frame = kInvalidFrameIndex;

  shader->global_ubo_data = {};
  shader->instance_ubo_data = {};

  if (!shader->uniform_buffers.IsEmpty()) {
    for (auto& buffer : shader->uniform_buffers) {
      if (IsBufferInitialized(buffer)) {
        DestroyBuffer(buffer);
      }
    }
  }

  shader->uniform_buffers.Destroy();

  if (shader->descriptor_pool_handle != VK_NULL_HANDLE) {
    DestroyDescriptorPool(device, shader->descriptor_pool_handle);
  }

  for (auto& instance : shader->instances.list) {
    DestroyBindingRuntimeData(instance.binding_data);
  }

  shader->instances.list.Destroy();
  shader->instances.indices.Destroy();

  for (u32 i{0}; i < shader->layout_bindings.count; ++i) {
    auto layout_handle{shader->layout_handles[i]};

    if (layout_handle != VK_NULL_HANDLE) {
      vkDestroyDescriptorSetLayout(device, layout_handle, VK_NULL_HANDLE);
      shader->layout_handles[i] = VK_NULL_HANDLE;
    }
  }

  shader->layout_bindings = {};
  shader->vertex_attributes.Destroy();

  for (auto& binding : shader->bindings) {
    binding.fields.Destroy();
  }

  for (auto& block : shader->push_constant_blocks) {
    block.fields.Destroy();
  }

  shader->bindings.Destroy();
  shader->push_constant_blocks.Destroy();

  if (shader_module_handler_->IsInitialized()) {
    for (auto& module : shader->modules) {
      shader_module_handler_->Destroy(module->id);
    }
  }

  shader->modules.Destroy();
  shader->push_constant_ranges.Destroy();
  shader_instance_allocator_.Deallocate(shader);
}

void ShaderHandler::DestroyBindingRuntimeData(
    ShaderBindingRuntimeData& binding_data) const {
  for (auto& runtime_binding : binding_data.image_bindings) {
    runtime_binding.descriptors.Destroy();
  }

  binding_data.image_bindings.Destroy();
}

void ShaderHandler::HandleShaderModulesGeneration(
    Shader* shader, const resource::ShaderResource* resource) {
  shader->modules.Reserve(resource->descr.shader_module_paths.GetSize());

  for (const auto& shader_module_path : resource->descr.shader_module_paths) {
    shader->modules.PushBack(
        shader_module_handler_->GetOrGenerate(shader_module_path));
  }
}

void ShaderHandler::HandleAttributesGeneration(
    Shader* shader, const resource::ShaderResource* resource) {
  COMET_ASSERT(shader != nullptr, "Shader is null!");
  COMET_ASSERT(resource != nullptr, "Shader resource is null!");

  u32 stride{0};
  PopulateVertexAttributes(resource->descr.vertex_layout,
                           shader->vertex_attributes, stride);
  shader->vertex_attribute_stride = static_cast<VertexAttributeStride>(stride);
}

void ShaderHandler::HandleBindingsGeneration(
    Shader* shader, const resource::ShaderResource* resource) {
  shader->bindings.Reserve(resource->descr.bindings.GetSize());

  for (const auto& binding_descr : resource->descr.bindings) {
    ShaderBinding binding{};
    binding.type = binding_descr.type;
    binding.scope = binding_descr.scope;
    binding.layout = binding_descr.layout;
    binding.stages = ResolveStageFlags(binding_descr.stages);
    binding.index = static_cast<ShaderBindingIndex>(shader->bindings.GetSize());
    binding.set = binding_descr.set;
    binding.binding = binding_descr.binding;
    binding.descriptor_count = binding_descr.descriptor_count;
    binding.fields = Array<ShaderField>{&general_allocator_};
    binding.image_semantic = binding_descr.image_semantic;

    binding.fields.Reserve(binding_descr.fields.GetSize());

    for (const auto& field_descr : binding_descr.fields) {
      ShaderField field{};
      field.type = field_descr.type;
      field.array_count = field_descr.array_count;
      binding.fields.PushBack(field);
    }

    ComputeBindingLayout(binding);

    if (binding.scope == ShaderBindingScope::Global &&
        binding.type == ShaderBindingType::UniformBuffer) {
      auto align{static_cast<ShaderOffset>(
          context_->GetDevice()
              .GetProperties()
              .limits.minUniformBufferOffsetAlignment)};

      shader->global_ubo_data.size =
          AlignOffset(shader->global_ubo_data.size, align);
      binding.offset = shader->global_ubo_data.size;
      shader->global_ubo_data.size += binding.size;
      shader->global_ubo_data.stages |= binding.stages;
    }

    if (binding.scope == ShaderBindingScope::Material &&
        binding.type == ShaderBindingType::UniformBuffer) {
      auto align{static_cast<ShaderOffset>(
          context_->GetDevice()
              .GetProperties()
              .limits.minUniformBufferOffsetAlignment)};

      shader->instance_ubo_data.size =
          AlignOffset(shader->instance_ubo_data.size, align);
      binding.offset = shader->instance_ubo_data.size;
      shader->instance_ubo_data.size += binding.size;
      shader->instance_ubo_data.stages |= binding.stages;
    }

    if (binding.type == ShaderBindingType::UniformBuffer &&
        binding.scope != ShaderBindingScope::Global &&
        binding.scope != ShaderBindingScope::Material) {
      COMET_ASSERT(false, "Pass/Draw uniform buffers are not implemented yet!");
    }

    shader->bindings.PushBack(std::move(binding));
  }
}

void ShaderHandler::HandlePushConstantBlocksGeneration(
    Shader* shader, const resource::ShaderResource* resource) {
  shader->push_constant_blocks.Reserve(
      resource->descr.push_constants.GetSize());
  shader->push_constant_ranges.Reserve(
      resource->descr.push_constants.GetSize());

  ShaderOffset push_constant_cursor{0};

  for (const auto& block_descr : resource->descr.push_constants) {
    ShaderPushConstantBlock block{};
    block.index = static_cast<ShaderPushConstantIndex>(
        shader->push_constant_blocks.GetSize());
    block.stages = ResolveStageFlags(block_descr.stages);
    block.fields = Array<ShaderField>{&general_allocator_};
    block.fields.Reserve(block_descr.fields.GetSize());

    for (const auto& field_descr : block_descr.fields) {
      ShaderField field{};
      field.type = field_descr.type;
      field.array_count = field_descr.array_count;
      block.fields.PushBack(field);
    }

    ComputePushConstantBlockLayout(block);

    push_constant_cursor = AlignOffset(push_constant_cursor, 4);
    block.offset = push_constant_cursor;
    push_constant_cursor += block.size;

    VkPushConstantRange range{};
    range.stageFlags = block.stages;
    range.offset = static_cast<u32>(block.offset);
    range.size = static_cast<u32>(block.size);

    shader->push_constant_ranges.PushBack(range);
    shader->push_constant_blocks.PushBack(std::move(block));
  }

#ifdef COMET_RENDERING_DEBUG
  auto max_push_constants{
      context_->GetDevice().GetProperties().limits.maxPushConstantsSize};
  COMET_ASSERT(push_constant_cursor <= max_push_constants,
               "Push constant data exceeds device limit!");
#endif  // COMET_RENDERING_DEBUG
}

void ShaderHandler::ComputeBindingLayout(ShaderBinding& binding) const {
  if (!IsBufferBindingType(binding.type) || binding.fields.IsEmpty()) {
    binding.size = 0;
    binding.stride = 0;
    return;
  }

  ShaderOffset cursor{0};

  for (ShaderFieldIndex i{0}; i < binding.fields.GetSize(); ++i) {
    auto& field{binding.fields[i]};
    auto info{
        GetFieldLayoutInfo(binding.layout, field.type, field.array_count)};

    cursor = AlignOffset(cursor, info.alignment);

    field.offset = cursor;
    field.element_size = info.element_size;
    field.aligned_size = info.aligned_size;
    field.stride = info.stride;
    field.size = info.total_size;

    cursor += field.size;
  }

  binding.size = cursor;
  binding.stride = cursor;
}

void ShaderHandler::ComputePushConstantBlockLayout(
    ShaderPushConstantBlock& block) const {
  ShaderOffset cursor{0};

  for (ShaderFieldIndex i{0}; i < block.fields.GetSize(); ++i) {
    auto& field{block.fields[i]};

    auto alignment{GetStd430Alignment(field.type)};
    auto element_size{GetShaderVariableTypeSize(field.type)};
    auto aligned_size{static_cast<ShaderVariableSize>(memory::AlignSize(
        element_size, static_cast<memory::Alignment>(alignment)))};

    cursor = AlignOffset(cursor, alignment);

    field.offset = cursor;
    field.element_size = element_size;
    field.aligned_size = aligned_size;
    field.stride = aligned_size;
    field.size =
        field.array_count > 1
            ? static_cast<ShaderVariableSize>(field.stride * field.array_count)
            : aligned_size;

    cursor += field.size;
  }

  block.size = cursor;
}

void ShaderHandler::HandleDescriptorSetLayoutsGeneration(Shader* shader) const {
  shader->layout_bindings.count = 0;

  for (const auto& binding : shader->bindings) {
    COMET_ASSERT(binding.set < kDescriptorSetMaxLayoutCount,
                 "Descriptor set index out of bounds!");

    auto& set_layout{shader->layout_bindings.list[binding.set]};
    COMET_ASSERT(set_layout.binding_count < kDescriptorBindingMaxCount,
                 "Too many bindings in descriptor set ", binding.set, "!");
    auto& vk_binding{set_layout.bindings[set_layout.binding_count++]};

    vk_binding.binding = binding.binding;
    vk_binding.descriptorCount = binding.descriptor_count;
    vk_binding.descriptorType = GetVkDescriptorType(binding.type);
    vk_binding.stageFlags = binding.stages;

    shader->layout_bindings.count =
        math::Max(shader->layout_bindings.count, binding.set + 1);
  }

  for (u32 i{0}; i < shader->layout_bindings.count; ++i) {
    auto info{init::GenerateDescriptorSetLayoutCreateInfo(
        shader->layout_bindings.list[i])};

    COMET_CHECK_VK(
        vkCreateDescriptorSetLayout(context_->GetDevice(), &info,
                                    VK_NULL_HANDLE, &shader->layout_handles[i]),
        "Could not create descriptor set layout for shader ",
        COMET_STRING_ID_LABEL(shader->handle), "!");
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

  if (HasVertexStage(shader)) {
    COMET_ASSERT(shader->vertex_layout != ShaderVertexLayout::None,
                 "Shader with a vertex stage requires a vertex layout!");
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
  GraphicsPipelineDescr pipeline_descr{};
  pipeline_descr.render_pass_handle = shader->render_pass_handle;
  pipeline_descr.layout_handle = pipeline_layout->handle;

  if (!shader->vertex_attributes.IsEmpty()) {
    pipeline_descr.vertex_input_binding_description.binding = 0;
    pipeline_descr.vertex_input_binding_description.stride =
        shader->vertex_attribute_stride;
    pipeline_descr.vertex_input_binding_description.inputRate =
        VK_VERTEX_INPUT_RATE_VERTEX;
    pipeline_descr.vertex_attributes = &shader->vertex_attributes;
  }

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
      init::GeneratePipelineInputAssemblyStateCreateInfo(shader->topology);
  pipeline_descr.rasterization_state =
      init::GeneratePipelineRasterizationStateCreateInfo(shader->rasterizer);
  pipeline_descr.color_blend_attachment_state =
      init::GeneratePipelineColorBlendAttachmentState();

  pipeline_descr.multisample_state =
      init::GeneratePipelineMultisampleStateCreateInfo();

  auto sample_count{
      shader->render_pass_handle != kInvalidRenderPassHandle
          ? render_pass_handler_->GetSamples(shader->render_pass_handle)
          : VK_SAMPLE_COUNT_1_BIT};

  pipeline_descr.multisample_state.rasterizationSamples = sample_count;

  auto is_multisampled{sample_count != VK_SAMPLE_COUNT_1_BIT};
  auto is_sample_rate_shading{is_multisampled &&
                              context_->IsSampleRateShading()};

  pipeline_descr.multisample_state.sampleShadingEnable =
      is_sample_rate_shading ? VK_TRUE : VK_FALSE;
  pipeline_descr.multisample_state.minSampleShading =
      is_sample_rate_shading ? .2f : .0f;

  pipeline_descr.depth_stencil_state =
      init::GeneratePipelineDepthStencilStateCreateInfo(shader->depth_stencil);

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
  constexpr VkDescriptorPoolSize pool_sizes[6]{
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1024},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4096},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4096},
      {VK_DESCRIPTOR_TYPE_SAMPLER, 1024},
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1024},
  };

  shader->descriptor_pool_handle =
      GenerateDescriptorPool(context_->GetDevice(), 1024, pool_sizes, 6,
                             VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

  shader->global_descriptor_data.descriptor_pool_handle =
      shader->descriptor_pool_handle;
  shader->storage_descriptor_data.descriptor_pool_handle =
      shader->descriptor_pool_handle;
}

void ShaderHandler::HandleUboBufferGeneration(Shader* shader) {
  auto align{context_->GetDevice()
                 .GetProperties()
                 .limits.minUniformBufferOffsetAlignment};

  shader->global_ubo_data.stride = static_cast<ShaderOffset>(memory::AlignSize(
      shader->global_ubo_data.size, static_cast<memory::Alignment>(align)));

  shader->instance_ubo_data.stride = static_cast<ShaderOffset>(
      memory::AlignSize(shader->instance_ubo_data.size,
                        static_cast<memory::Alignment>(align)));

#ifdef COMET_DEBUG
  auto max_range{
      context_->GetDevice().GetProperties().limits.maxUniformBufferRange};
  COMET_ASSERT(shader->global_ubo_data.stride < max_range,
               "Shader global UBO data is too big!");
  COMET_ASSERT(shader->instance_ubo_data.stride < max_range,
               "Shader instance UBO data is too big!");
#endif

  auto buffer_size{static_cast<VkDeviceSize>(shader->global_ubo_data.stride +
                                             shader->instance_ubo_data.stride *
                                                 kMaxMaterialInstances)};

  if (buffer_size == 0) {
    return;
  }

  shader->uniform_buffers = Array<Buffer>{&general_allocator_};
  shader->uniform_buffers.Resize(context_->GetImageCount());

  for (usize i{0}; i < shader->uniform_buffers.GetSize(); ++i) {
    shader->uniform_buffers[i] = GenerateBuffer(
        context_->GetAllocatorHandle(), buffer_size,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0,
        VK_SHARING_MODE_EXCLUSIVE, "shader->uniform_buffers");
  }

  shader->global_ubo_data.offset = 0;
  shader->bound_global_ubo_offset = 0;
}

void ShaderHandler::AllocateGlobalDescriptorSets(Shader* shader) {
  if (!internal::HasBindingInSet(shader, internal::kGlobalSet)) {
    return;
  }

  frame::FrameArray<VkDescriptorSetLayout> layout_buffer{};
  shader->global_descriptor_data.descriptor_set_handles.Resize(
      descriptor_set_layout_count_);

  for (u32 i{0}; i < descriptor_set_layout_count_; ++i) {
    layout_buffer.PushBack(shader->layout_handles[internal::kGlobalSet]);
  }

  [[maybe_unused]] auto is_allocated{descriptor_handler_->Generate(
      layout_buffer.GetData(),
      shader->global_descriptor_data.descriptor_set_handles.GetData(),
      descriptor_set_layout_count_, DescriptorType::Static)};

  COMET_ASSERT(is_allocated, "Failed to allocate global descriptor sets!");

  for (u32 image_index{0}; image_index < descriptor_set_layout_count_;
       ++image_index) {
    auto set_handle{
        shader->global_descriptor_data.descriptor_set_handles[image_index]};

    for (const auto& binding : shader->bindings) {
      if (binding.set != internal::kGlobalSet ||
          binding.type != ShaderBindingType::UniformBuffer) {
        continue;
      }

      UpdateDescriptorSetBuffer(
          set_handle, binding, shader->uniform_buffers[image_index].handle,
          binding.size, shader->global_ubo_data.offset + binding.offset);
    }
  }
}

void ShaderHandler::AllocateStorageDescriptorSets(Shader* shader) {
  if (!internal::HasBindingInSet(shader, internal::kPassSet)) {
    return;
  }

  frame::FrameArray<VkDescriptorSetLayout> layout_buffer{};
  shader->storage_descriptor_data.descriptor_set_handles.Resize(
      descriptor_set_layout_count_);

  for (u32 i{0}; i < descriptor_set_layout_count_; ++i) {
    layout_buffer.PushBack(shader->layout_handles[internal::kPassSet]);
  }

  [[maybe_unused]] auto is_allocated{descriptor_handler_->Generate(
      layout_buffer.GetData(),
      shader->storage_descriptor_data.descriptor_set_handles.GetData(),
      descriptor_set_layout_count_, DescriptorType::Static)};

  COMET_ASSERT(is_allocated,
               "Failed to allocate storage/pass descriptor sets!");

  // Storage/pass descriptor sets are only allocated here.
  // Actual buffer bindings are provided later by the rendering pass, since
  // those buffers are pass-owned and may vary per frame.
}

void ShaderHandler::AllocateMaterialDescriptorSets(Shader* shader,
                                                   MaterialInstance& instance) {
  if (!internal::HasBindingInSet(shader, internal::kMaterialSet)) {
    return;
  }

  frame::FrameArray<VkDescriptorSetLayout> layout_buffer{};
  instance.descriptor_data.descriptor_set_handles.Resize(
      descriptor_set_layout_count_);
  instance.descriptor_data.descriptor_pool_handle =
      shader->descriptor_pool_handle;

  for (u32 i{0}; i < descriptor_set_layout_count_; ++i) {
    layout_buffer.PushBack(shader->layout_handles[internal::kMaterialSet]);
  }

  [[maybe_unused]] auto is_allocated{descriptor_handler_->Generate(
      layout_buffer.GetData(),
      instance.descriptor_data.descriptor_set_handles.GetData(),
      descriptor_set_layout_count_, DescriptorType::Static)};

  COMET_ASSERT(is_allocated, "Failed to allocate material descriptor sets!");
}

void ShaderHandler::CollectMaterialTextureMaps(
    const Material* material, const ShaderBinding& binding,
    Array<const TextureMap*>& texture_maps) const {
  COMET_ASSERT(material != nullptr, "Material is null!");
  COMET_ASSERT(IsImageBindingType(binding.type),
               "Binding is not an image binding!");

  texture_maps.Clear();
  texture_maps.Reserve(binding.descriptor_count);

  switch (binding.image_semantic) {
    case ShaderImageBindingSemantic::MaterialDiffuse:
      COMET_ASSERT(binding.descriptor_count == 1,
                   "MaterialDiffuse binding must have descriptor_count == 1!");
      texture_maps.PushBack(&material->diffuse_map);
      return;

    case ShaderImageBindingSemantic::MaterialSpecular:
      COMET_ASSERT(binding.descriptor_count == 1,
                   "MaterialSpecular binding must have descriptor_count == 1!");
      texture_maps.PushBack(&material->specular_map);
      return;

    case ShaderImageBindingSemantic::MaterialNormal:
      COMET_ASSERT(binding.descriptor_count == 1,
                   "MaterialNormal binding must have descriptor_count == 1!");
      texture_maps.PushBack(&material->normal_map);
      return;

    case ShaderImageBindingSemantic::MaterialTextures:
      COMET_ASSERT(binding.descriptor_count == 3,
                   "MaterialTextures binding must have descriptor_count == 3!");
      texture_maps.PushBack(&material->diffuse_map);
      texture_maps.PushBack(&material->specular_map);
      texture_maps.PushBack(&material->normal_map);
      return;

    default:
      COMET_ASSERT(false, "Unsupported material image binding semantic!");
      return;
  }
}

void ShaderHandler::CollectMaterialImageDescriptors(
    const Material* material, const ShaderBinding& binding,
    Array<ShaderImageDescriptor>& descriptors) const {
  auto& texture_maps{*COMET_FRAME_ARRAY(const TextureMap*)};
  CollectMaterialTextureMaps(material, binding, texture_maps);
  GenerateImageDescriptors(binding.type, texture_maps, descriptors);
}

void ShaderHandler::UpdateDescriptorSetImages(
    VkDescriptorSet set_handle, const ShaderBinding& binding,
    const ShaderImageDescriptor* descriptors, u32 descriptor_count) const {
  COMET_ASSERT(descriptors != nullptr, "Image descriptors are null!");
  COMET_ASSERT(descriptor_count > 0, "Image descriptor count is 0!");
  COMET_ASSERT(descriptor_count == binding.descriptor_count,
               "Descriptor count does not match binding descriptor count!");
  COMET_ASSERT(IsImageBindingType(binding.type),
               "Tried to update image descriptors for a non-image binding!");

  frame::FrameArray<VkDescriptorImageInfo> image_infos{};
  image_infos.Reserve(descriptor_count);

  for (u32 i{0}; i < descriptor_count; ++i) {
    const auto& descriptor{descriptors[i]};
    VkDescriptorImageInfo image_info{};

    switch (binding.type) {
      case ShaderBindingType::CombinedImageSampler:
        COMET_ASSERT(descriptor.texture != nullptr,
                     "CombinedImageSampler requires a texture!");
        COMET_ASSERT(descriptor.sampler != nullptr,
                     "CombinedImageSampler requires a sampler!");
        image_info.imageView = descriptor.texture->image.image_view_handle;
        image_info.sampler = descriptor.sampler->handle;
        image_info.imageLayout = descriptor.image_layout;
        break;

      case ShaderBindingType::SampledImage:
        COMET_ASSERT(descriptor.texture != nullptr,
                     "SampledImage requires a texture!");
        image_info.imageView = descriptor.texture->image.image_view_handle;
        image_info.sampler = VK_NULL_HANDLE;
        image_info.imageLayout = descriptor.image_layout;
        break;

      case ShaderBindingType::Sampler:
        COMET_ASSERT(descriptor.sampler != nullptr,
                     "Sampler binding requires a sampler!");
        image_info.imageView = VK_NULL_HANDLE;
        image_info.sampler = descriptor.sampler->handle;
        image_info.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        break;

      case ShaderBindingType::StorageImage:
        COMET_ASSERT(descriptor.texture != nullptr,
                     "StorageImage requires a texture!");
        image_info.imageView = descriptor.texture->image.image_view_handle;
        image_info.sampler = VK_NULL_HANDLE;
        image_info.imageLayout = descriptor.image_layout;
        break;

      default:
        COMET_ASSERT(false, "Unsupported image binding type!");
        break;
    }

    image_infos.PushBack(image_info);
  }

  auto write{init::GenerateImageWriteDescriptorSet(
      GetVkDescriptorType(binding.type), set_handle, image_infos.GetData(),
      descriptor_count, binding.binding)};

  vkUpdateDescriptorSets(context_->GetDevice(), 1, &write, 0, VK_NULL_HANDLE);
}

void ShaderHandler::UpdateDescriptorSetBuffer(VkDescriptorSet set_handle,
                                              const ShaderBinding& binding,
                                              VkBuffer buffer_handle,
                                              VkDeviceSize range,
                                              VkDeviceSize offset) const {
  COMET_ASSERT(
      binding.type == ShaderBindingType::UniformBuffer ||
          binding.type == ShaderBindingType::StorageBuffer,
      "Tried to update descriptor set buffer for a non-buffer binding!");
  COMET_ASSERT(buffer_handle != VK_NULL_HANDLE,
               "Tried to update descriptor set with null buffer!");

  auto buffer_info{
      init::GenerateDescriptorBufferInfo(buffer_handle, offset, range)};

  auto write{init::GenerateBufferWriteDescriptorSet(
      GetVkDescriptorType(binding.type), set_handle, &buffer_info,
      binding.binding)};

  vkUpdateDescriptorSets(context_->GetDevice(), 1, &write, 0, VK_NULL_HANDLE);
}

void ShaderHandler::WriteBindingFieldToUbo(
    Buffer& buffer, ShaderOffset base_offset, const ShaderBinding& binding,
    ShaderFieldIndex field_index, const void* value, usize value_size) const {
  ScopedMappedBuffer mapped{buffer};
  WriteBindingFieldToMappedUbo(buffer, base_offset, binding, field_index, value,
                               value_size);
}

void ShaderHandler::WriteBindingFieldToMappedUbo(
    Buffer& buffer, ShaderOffset base_offset, const ShaderBinding& binding,
    ShaderFieldIndex field_index, const void* value, usize value_size) const {
  COMET_ASSERT(field_index < binding.fields.GetSize(),
               "Invalid field index for binding!");

  const auto& field{binding.fields[field_index]};
  COMET_ASSERT(value != nullptr, "Field value is null!");

  auto raw_size{static_cast<usize>(field.element_size)};
  auto copy_size{value_size == 0 ? raw_size : value_size};

  COMET_ASSERT(copy_size <= field.size,
               "Field update size exceeds field size!");

  CopyToBuffer(buffer, value, copy_size,
               base_offset + binding.offset + field.offset);
}

void ShaderHandler::UpdateInstanceUboDescriptors(Shader* shader,
                                                 MaterialInstance& instance,
                                                 u32 instance_index) {
  COMET_ASSERT(shader != nullptr, "Shader is null!");
  COMET_ASSERT(instance.descriptor_data.descriptor_set_handles.GetSize() ==
                   descriptor_set_layout_count_,
               "Invalid material descriptor set handle count!");

  instance.offset = shader->instance_ubo_data.stride * instance_index;

  bool has_material_ubo_binding{false};

  for (const auto& binding : shader->bindings) {
    if (binding.set == internal::kMaterialSet &&
        binding.type == ShaderBindingType::UniformBuffer) {
      has_material_ubo_binding = true;
      break;
    }
  }

  if (!has_material_ubo_binding) {
    return;
  }

  COMET_ASSERT(
      shader->uniform_buffers.GetSize() == descriptor_set_layout_count_,
      "Invalid uniform buffer count!");

  for (u32 image_index{0}; image_index < descriptor_set_layout_count_;
       ++image_index) {
    auto set_handle{
        instance.descriptor_data.descriptor_set_handles[image_index]};

    for (const auto& binding : shader->bindings) {
      if (binding.set != internal::kMaterialSet) {
        continue;
      }

      if (binding.type != ShaderBindingType::UniformBuffer) {
        continue;
      }

      UpdateDescriptorSetBuffer(
          set_handle, binding, shader->uniform_buffers[image_index].handle,
          binding.size,
          shader->global_ubo_data.stride + instance.offset + binding.offset);
    }
  }
}

ShaderBindingImageRuntimeData* ShaderHandler::FindBindingRuntimeData(
    ShaderBindingRuntimeData& binding_data,
    ShaderBindingIndex binding_index) const {
  for (auto& runtime_binding : binding_data.image_bindings) {
    if (runtime_binding.binding_index == binding_index) {
      return &runtime_binding;
    }
  }

  return nullptr;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet
