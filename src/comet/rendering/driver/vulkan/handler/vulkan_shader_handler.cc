// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_shader_handler.h"

#include <functional>

#include "comet/core/c_string.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"
#include "comet/rendering/driver/vulkan/data/vulkan_buffer.h"
#include "comet/rendering/driver/vulkan/data/vulkan_pipeline.h"
#include "comet/rendering/driver/vulkan/data/vulkan_shader_data.h"
#include "comet/rendering/driver/vulkan/data/vulkan_texture_map.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_buffer_utils.h"
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
      texture_handler_{descr.texture_handler} {
  COMET_ASSERT(shader_module_handler_ != nullptr,
               "Shader module handler is null!");
  COMET_ASSERT(pipeline_handler_ != nullptr, "Pipeline handler is null!");
  COMET_ASSERT(texture_handler_ != nullptr, "Texture handler is null!");
}

void ShaderHandler::Initialize() {
  Handler::Initialize();
  descriptor_set_layout_handles_buffer_.resize(context_->GetImageCount());
}

void ShaderHandler::Shutdown() {
  descriptor_set_layout_handles_buffer_.clear();

  for (auto& it : shaders_) {
    Destroy(it.second, true);
  }

  shaders_.clear();
  bound_shader_ = nullptr;
  instance_id_handler_.Shutdown();
  Handler::Shutdown();
}

Shader* ShaderHandler::Generate(const ShaderDescr& descr) {
  const auto* shader_resource{
      resource::ResourceManager::Get().Load<resource::ShaderResource>(
          descr.resource_path)};
  COMET_ASSERT(shader_resource != nullptr, "Shader resource is null!");
  Shader shader{};
  shader.id = shader_resource->id;
  shader.render_pass = descr.render_pass;
  shader.is_wireframe = shader_resource->descr.is_wireframe;
  shader.cull_mode = shader_resource->descr.cull_mode;

  HandleShaderModulesGeneration(shader, *shader_resource);
  HandleAttributesGeneration(shader, *shader_resource);
  HandleUniformsGeneration(shader, *shader_resource);
  HandleBindingsGeneration(shader);
  HandleDescriptorSetLayoutsGeneration(shader);
  HandlePipelineGeneration(shader);
  HandleDescriptorPoolGeneration(shader);
  HandleBufferGeneration(shader);

#ifdef COMET_DEBUG
  const auto shader_id{shader.id};
#endif  // COMET_DEBUG
  auto insert_pair{shaders_.emplace(shader.id, std::move(shader))};
  COMET_ASSERT(insert_pair.second,
               "Could not insert shader: ", COMET_STRING_ID_LABEL(shader_id),
               "!");
  return &insert_pair.first->second;
}

Shader* ShaderHandler::Get(ShaderId shader_id) {
  auto* shader{TryGet(shader_id)};
  COMET_ASSERT(shader != nullptr, "Requested shader does not exist: ",
               COMET_STRING_ID_LABEL(shader_id), "!");
  return shader;
}

Shader* ShaderHandler::TryGet(ShaderId shader_id) {
  auto it{shaders_.find(shader_id)};

  if (it == shaders_.end()) {
    return nullptr;
  }

  return &it->second;
}

void ShaderHandler::Destroy(ShaderId shader_id) { Destroy(*Get(shader_id)); }

void ShaderHandler::Destroy(Shader& shader) { Destroy(shader, false); }

void ShaderHandler::Bind(Shader& shader) {
  if (&shader == bound_shader_) {
    return;
  }

  COMET_ASSERT(shader.pipeline != nullptr, "Pipeline cannot be null!");
  pipeline_handler_->Bind(*shader.pipeline);
  bound_shader_ = &shader;
}

void ShaderHandler::BindGlobal(Shader& shader) const {
  shader.bound_ubo_offset = shader.global_ubo_data.ubo_offset;
}

void ShaderHandler::BindInstance(Shader& shader,
                                 MaterialInstanceId instance_id) const {
  shader.bound_instance_index = GetInstanceIndex(shader, instance_id);
  shader.bound_ubo_offset =
      shader.global_ubo_data.ubo_stride +
      shader.instances.list[shader.bound_instance_index].offset;
}

void ShaderHandler::Reset() { bound_shader_ = nullptr; }

void ShaderHandler::UpdateGlobal(Shader& shader,
                                 const ShaderPacket& packet) const {
  auto frame_count{context_->GetFrameCount()};

  if (shader.global_uniform_data.update_frame == frame_count) {
    return;
  }

  BindGlobal(shader);
  SetUniform(shader, shader.uniform_indices.projection,
             packet.projection_matrix);
  SetUniform(shader, shader.uniform_indices.view, packet.view_matrix);

  VkDescriptorSet global_descriptor_set_handle{
      shader.global_uniform_data
          .descriptor_set_handles[context_->GetImageIndex()]};

  auto buffer_info{init::GenerateDescriptorBufferInfo(
      shader.uniform_buffer.handle, shader.global_ubo_data.ubo_offset,
      shader.global_ubo_data.ubo_stride)};

  auto ubo_write_descriptor_set{init::GenerateBufferWriteDescriptorSet(
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, global_descriptor_set_handle,
      &buffer_info, 0)};
  const auto binding_count{
      shader.layout_bindings.list[kShaderDescriptorSetGlobalIndex]
          .binding_count};

  vkUpdateDescriptorSets(context_->GetDevice(), binding_count,
                         &ubo_write_descriptor_set, 0, VK_NULL_HANDLE);
  vkCmdBindDescriptorSets(context_->GetFrameData().command_buffer_handle,
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          shader.pipeline->layout_handle,
                          kShaderDescriptorSetGlobalIndex, binding_count,
                          &global_descriptor_set_handle, 0, VK_NULL_HANDLE);

  shader.global_uniform_data.update_frame = frame_count;
}

void ShaderHandler::UpdateLocal(Shader& shader,
                                const ShaderLocalPacket& packet) {
  SetUniform(shader, shader.uniform_indices.model, packet.position);
}

void ShaderHandler::SetUniform(Shader& shader, const ShaderUniform& uniform,
                               const void* value) const {
  if (uniform.type == ShaderUniformType::Sampler) {
    if (uniform.scope == ShaderUniformScope::Global) {
      shader.global_uniform_data.texture_maps[uniform.location] =
          static_cast<const TextureMap*>(value);
    } else {
      shader.instances.list[shader.bound_instance_index]
          .uniform_data.texture_maps[uniform.location] =
          static_cast<const TextureMap*>(value);
    }

    return;
  }

  if (uniform.scope == ShaderUniformScope::Local) {
    vkCmdPushConstants(
        context_->GetFrameData().command_buffer_handle,
        shader.pipeline->layout_handle,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        uniform.offset, uniform.size, value);
    return;
  }

  // TODO(m4jr0): Use persistent mapping? {Unm, M}apping is expensive.
  auto& buffer{shader.uniform_buffer};
  MapBuffer(buffer);
  CopyToBuffer(buffer, value, uniform.size,
               shader.bound_ubo_offset + uniform.offset);
  UnmapBuffer(buffer);
}

void ShaderHandler::SetUniform(ShaderId id, const ShaderUniform& uniform,
                               const void* value) {
  auto* shader{Get(id)};
  COMET_ASSERT(shader != nullptr, "Tried to retrieve shader pass from ID ", id,
               ", but instance is null!");
  SetUniform(*shader, uniform, value);
}

void ShaderHandler::SetUniform(Shader& shader, ShaderUniformLocation index,
                               const void* value) const {
  COMET_ASSERT(
      index < shader.uniforms.size(), "Tried to set uniform at index #", index,
      " for shader pass, but uniform count is ", shader.uniforms.size(), "!");
  SetUniform(shader, shader.uniforms[index], value);
}

void ShaderHandler::SetUniform(ShaderId id, ShaderUniformLocation index,
                               const void* value) {
  auto* shader{Get(id)};
  COMET_ASSERT(shader != nullptr, "Tried to retrieve shader pass from ID ", id,
               ", but instance is null!");
  SetUniform(*shader, index, value);
}

void ShaderHandler::BindMaterial(Material& material) {
  auto* shader{Get(material.shader_id)};

  MaterialInstance instance{};
  auto& descriptor_set{
      shader->layout_bindings.list[kShaderDescriptorSetInstanceIndex]};
  auto sampler_binding_index{descriptor_set.sampler_binding_index};
  auto instance_texture_count{
      descriptor_set.bindings[sampler_binding_index].descriptorCount};

  if (instance_texture_count > 0) {
    instance.uniform_data.texture_maps.resize(instance_texture_count);

    // TODO(m4jr0): Put texture maps in generic array.
    StaticArray<TextureMap*, 3> maps{
        &material.diffuse_map, &material.specular_map, &material.normal_map};
    memory::CopyMemory(instance.uniform_data.texture_maps.data(),
                       maps.GetData(), sizeof(TextureMap*) * maps.GetSize());
  }

  const auto layout_count{descriptor_set_layout_handles_buffer_.size()};
  instance.uniform_data.descriptor_set_handles.resize(layout_count);
  instance.uniform_data.descriptor_pool_handle = shader->descriptor_pool_handle;

  for (u32 i{0}; i < layout_count; ++i) {
    descriptor_set_layout_handles_buffer_[i] =
        shader->layout_handles[kShaderDescriptorSetInstanceIndex];
  }

  AllocateShaderUniformData(context_->GetDevice(),
                            descriptor_set_layout_handles_buffer_,
                            instance.uniform_data);
  instance.offset =
      shader->instance_ubo_data.ubo_stride * shader->instances.list.size();

  // Generate material instance.
  shader->instances.list.push_back(std::move(instance));
  material.instance_id = instance_id_handler_.Generate();
  instance.id = material.instance_id;
  shader->instances.ids.push_back(material.instance_id);
}

void ShaderHandler::UnbindMaterial(Material& material) {
  COMET_ASSERT(HasMaterial(material),
               "Trying to destroy dead material instance #",
               material.instance_id, "!");

  auto* shader{Get(material.shader_id)};
  auto index{GetInstanceIndex(*shader, material)};
  auto& instances{shader->instances.list};

  auto& instance{instances[index]};
  context_->GetDevice().WaitIdle();
  FreeShaderUniformData(context_->GetDevice(), instance.uniform_data);
  instance.uniform_data.texture_maps.clear();

  // Delete material instance.
  // Copy last instance in place of the deleted one + Remove last duplicate in
  // the list.
  auto& instance_ids{shader->instances.ids};
  COMET_ASSERT(instance_ids.size() == instances.size(),
               "Size mismatch between material IDs (", instance_ids.size(),
               ") and instances (", instances.size(),
               ")! Something very wrong must have happened!");
  auto max_index{instance_ids.size() - 1};

  if (index != max_index) {
    instance_ids[index] = instance_ids[max_index];
    instances[index] = instances[max_index];
  }

  instance_ids.resize(max_index);
  instances.resize(max_index);
  instance_id_handler_.Destroy(material.instance_id);
  material.instance_id = kInvalidMaterialInstanceId;
}

bool ShaderHandler::HasMaterial(const Material& material) const {
  return instance_id_handler_.IsAlive(material.instance_id);
}

MaterialInstance& ShaderHandler::GetInstance(Shader& shader,
                                             const Material& material) {
  return shader.instances.list[GetInstanceIndex(shader, material)];
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
          static_cast<std::underlying_type_t<ShaderUniformType>>(type), "!");
  }

  return kInvalidShaderVertexAttributeSize;
}

ShaderUniformSize ShaderHandler::GetUniformSize(ShaderUniformType type) {
  switch (type) {
    case ShaderUniformType::B32:
    case ShaderUniformType::S32:
    case ShaderUniformType::U32:
    case ShaderUniformType::F32:
      return 4;

    case ShaderUniformType::F64:
      return 8;

    case ShaderUniformType::B32Vec2:
    case ShaderUniformType::S32Vec2:
    case ShaderUniformType::U32Vec2:
    case ShaderUniformType::Vec2:
      return 8;

    case ShaderUniformType::B32Vec3:
    case ShaderUniformType::S32Vec3:
    case ShaderUniformType::U32Vec3:
    case ShaderUniformType::Vec3:
      return 12;

    case ShaderUniformType::B32Vec4:
    case ShaderUniformType::S32Vec4:
    case ShaderUniformType::U32Vec4:
    case ShaderUniformType::Vec4:
      return 16;

    case ShaderUniformType::F64Vec2:
      return 16;

    case ShaderUniformType::F64Vec3:
      return 24;

    case ShaderUniformType::F64Vec4:
      return 32;

    case ShaderUniformType::Mat2x2:
      return 8;

    case ShaderUniformType::Mat2x3:
    case ShaderUniformType::Mat3x2:
      return 24;

    case ShaderUniformType::Mat3x3:
      return 36;

    case ShaderUniformType::Mat2x4:
    case ShaderUniformType::Mat4x2:
      return 32;

    case ShaderUniformType::Mat3x4:
    case ShaderUniformType::Mat4x3:
      return 48;

    case ShaderUniformType::Mat4x4:
      return 64;

    case ShaderUniformType::Sampler:
    case ShaderUniformType::Image:
    case ShaderUniformType::Atomic:
      return kInvalidShaderUniformSize;

    default:
      COMET_ASSERT(false, "Unknown or unsupported shader data type: ",
                   static_cast<std::underlying_type_t<ShaderUniformType>>(type),
                   "!");
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
          static_cast<std::underlying_type_t<ShaderUniformType>>(type), "!");
  }

  return VK_FORMAT_UNDEFINED;
}

void ShaderHandler::Destroy(Shader& shader, bool is_destroying_handler) {
  shader.is_wireframe = false;
  shader.cull_mode = CullMode::Unknown;
  shader.id = kInvalidShaderId;
  shader.vertex_attribute_stride = 0;
  shader.bound_ubo_offset = 0;
  shader.bound_instance_index = kInvalidMaterialInstanceId;
  shader.global_uniform_data = {};
  shader.global_ubo_data.uniform_count = 0;
  shader.global_ubo_data.sampler_count = 0;
  shader.global_ubo_data.ubo_size = 0;
  shader.global_ubo_data.ubo_stride = 0;
  shader.global_ubo_data.ubo_offset = 0;
  shader.instance_ubo_data.uniform_count = 0;
  shader.instance_ubo_data.sampler_count = 0;
  shader.instance_ubo_data.ubo_size = 0;
  shader.instance_ubo_data.ubo_stride = 0;
  shader.instance_ubo_data.ubo_offset = 0;
  DestroyBuffer(shader.uniform_buffer);
  DestroyDescriptorPool(context_->GetDevice(), shader.descriptor_pool_handle);
  shader.render_pass = nullptr;
  shader.pipeline = nullptr;
  shader.instances.ids.clear();
  shader.instances.list.clear();
  const auto& device{context_->GetDevice()};

  for (u32 i{0}; i < shader.layout_bindings.count; ++i) {
    vkDestroyDescriptorSetLayout(device, shader.layout_handles[i],
                                 VK_NULL_HANDLE);
    shader.layout_handles[i] = VK_NULL_HANDLE;
  }

  shader.layout_bindings = {};
  shader.vertex_attributes.clear();
  shader.uniforms.clear();
  shader.uniform_indices.projection = kInvalidShaderUniformIndex;
  shader.uniform_indices.view = kInvalidShaderUniformIndex;
  shader.uniform_indices.ambient_color = kInvalidShaderUniformIndex;
  shader.uniform_indices.view_pos = kInvalidShaderUniformIndex;
  shader.uniform_indices.diffuse_color = kInvalidShaderUniformIndex;
  shader.uniform_indices.diffuse_map = kInvalidShaderUniformIndex;
  shader.uniform_indices.specular_map = kInvalidShaderUniformIndex;
  shader.uniform_indices.normal_map = kInvalidShaderUniformIndex;
  shader.uniform_indices.shininess = kInvalidShaderUniformIndex;
  shader.uniform_indices.model = kInvalidShaderUniformIndex;

  if (shader_module_handler_->IsInitialized()) {
    for (auto& module : shader.modules) {
      shader_module_handler_->Destroy(module->id);
    }
  }

  shader.modules.clear();
  shader.push_constant_ranges.clear();

  if (!is_destroying_handler) {
    shaders_.erase(shader.id);
  }
}

u32 ShaderHandler::GetInstanceIndex(const Shader& shader,
                                    const Material& material) const {
  return GetInstanceIndex(shader, material.instance_id);
}

u32 ShaderHandler::GetInstanceIndex(
    const Shader& shader, const MaterialInstanceId instance_id) const {
  auto& instance_ids{shader.instances.ids};
  auto it{std::find(instance_ids.begin(), instance_ids.end(), instance_id)};
  COMET_ASSERT(it != instance_ids.end(),
               "Unable to find bound material instance (ID: ", instance_id,
               ") in shader ", COMET_STRING_ID_LABEL(shader.id), "!");
  return static_cast<u32>(it - instance_ids.begin());
}

void ShaderHandler::HandleShaderModulesGeneration(
    Shader& shader, const resource::ShaderResource& resource) const {
  for (const auto& shader_module_path : resource.descr.shader_module_paths) {
    shader.modules.push_back(
        shader_module_handler_->GetOrGenerate(shader_module_path));
  }
}

void ShaderHandler::HandleAttributesGeneration(
    Shader& shader, const resource::ShaderResource& resource) const {
  auto vertex_attribute_count{
      static_cast<u32>(resource.descr.vertex_attributes.GetSize())};
  u32 offset{0};
  shader.vertex_attributes.reserve(vertex_attribute_count);

  for (u32 i{0}; i < resource.descr.vertex_attributes.GetSize(); ++i) {
    VkVertexInputAttributeDescription vertex_attribute{};
    const auto& vertex_attribute_descr{resource.descr.vertex_attributes[i]};
    vertex_attribute.location = i;
    vertex_attribute.binding = 0;
    vertex_attribute.offset = offset;
    vertex_attribute.format = GetVkFormat(vertex_attribute_descr.type);
    offset += GetVertexAttributeSize(vertex_attribute_descr.type);
    shader.vertex_attributes.push_back(vertex_attribute);
  }

  shader.vertex_attribute_stride = offset;
}

void ShaderHandler::HandleUniformsGeneration(
    Shader& shader, const resource::ShaderResource& resource) const {
  u32 instance_texture_count{0};

  for (const auto& uniform_descr : resource.descr.uniforms) {
    if (uniform_descr.type == ShaderUniformType::Sampler) {
      HandleSamplerGeneration(shader, uniform_descr, instance_texture_count);
      continue;
    }

    AddUniform(shader, uniform_descr, 0);
  }
}

void ShaderHandler::HandleBindingsGeneration(Shader& shader) const {
  shader.global_ubo_data.uniform_count = 0;
  shader.global_ubo_data.sampler_count = 0;
  shader.instance_ubo_data.uniform_count = 0;
  shader.instance_ubo_data.sampler_count = 0;

  for (const auto& uniform : shader.uniforms) {
    if (uniform.scope == ShaderUniformScope::Global) {
      if (uniform.type == ShaderUniformType::Sampler) {
        ++shader.global_ubo_data.sampler_count;
      } else {
        ++shader.global_ubo_data.uniform_count;
      }
    } else if (uniform.scope == ShaderUniformScope::Instance) {
      if (uniform.type == ShaderUniformType::Sampler) {
        ++shader.instance_ubo_data.sampler_count;
      } else {
        ++shader.instance_ubo_data.uniform_count;
      }
    } else if (uniform.scope == ShaderUniformScope::Local) {
      // TODO(m4jr0).
    } else {
      COMET_ASSERT(false, "Unknown or unsupported uniform scope: ",
                   static_cast<std::underlying_type_t<ShaderUniformScope>>(
                       uniform.scope),
                   "!");
    }
  }

  if (shader.global_ubo_data.uniform_count > 0 ||
      shader.global_ubo_data.sampler_count > 0) {
    auto& descriptor_set_data{
        shader.layout_bindings.list[shader.layout_bindings.count]};

    if (shader.global_ubo_data.uniform_count > 0) {
      auto& descriptor_set{
          descriptor_set_data.bindings[descriptor_set_data.binding_count]};
      descriptor_set.binding = descriptor_set_data.binding_count;
      descriptor_set.descriptorCount = 1;
      descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptor_set.stageFlags =
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
      descriptor_set_data.binding_count++;
    }

    if (shader.global_ubo_data.sampler_count > 0) {
      auto& descriptor_set{
          descriptor_set_data.bindings[descriptor_set_data.binding_count]};

      descriptor_set.binding = descriptor_set_data.binding_count;
      descriptor_set.descriptorCount = shader.global_ubo_data.sampler_count;
      descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      descriptor_set.stageFlags =
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
      descriptor_set_data.sampler_binding_index =
          descriptor_set_data.binding_count;
      descriptor_set_data.binding_count++;
    }

    shader.layout_bindings.count++;
  }

  if (shader.instance_ubo_data.uniform_count > 0 ||
      shader.instance_ubo_data.sampler_count > 0) {
    auto& descriptor_set_data{
        shader.layout_bindings.list[shader.layout_bindings.count]};

    if (shader.instance_ubo_data.uniform_count > 0) {
      auto& descriptor_set{
          descriptor_set_data.bindings[descriptor_set_data.binding_count]};
      descriptor_set.binding = descriptor_set_data.binding_count;
      descriptor_set.descriptorCount = 1;
      descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptor_set.stageFlags =
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
      descriptor_set_data.binding_count++;
    }

    if (shader.instance_ubo_data.sampler_count > 0) {
      auto& descriptor_set{
          descriptor_set_data.bindings[descriptor_set_data.binding_count]};

      descriptor_set.binding = descriptor_set_data.binding_count;
      descriptor_set.descriptorCount = shader.instance_ubo_data.sampler_count;
      descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      descriptor_set.stageFlags =
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
      descriptor_set_data.sampler_binding_index =
          descriptor_set_data.binding_count;
      descriptor_set_data.binding_count++;
    }

    shader.layout_bindings.count++;
  }
}

void ShaderHandler::HandleSamplerGeneration(
    Shader& shader, const ShaderUniformDescr& uniform_descr,
    u32& instance_texture_count) const {
  COMET_ASSERT(uniform_descr.scope != ShaderUniformScope::Local,
               "Samplers cannot be at a local scope!");
  auto location{kInvalidShaderUniformLocation};

  if (uniform_descr.scope == ShaderUniformScope::Global) {
    COMET_ASSERT(shader.global_uniform_data.texture_maps.size() ==
                     kMaxShaderTextureMapCount,
                 "Max texture map count reached: ", kMaxShaderTextureMapCount,
                 " for shader ", COMET_STRING_ID_LABEL(shader.id), "!");

    location = static_cast<ShaderUniformLocation>(
        shader.global_uniform_data.texture_maps.size() - 1);
    shader.global_uniform_data.texture_maps.resize(
        shader.global_uniform_data.texture_maps.size() + 1);
  } else {
    COMET_ASSERT(instance_texture_count <= kMaxShaderTextureMapCount,
                 "Max texture map count reached: ", kMaxShaderTextureMapCount,
                 " for shader ", COMET_STRING_ID_LABEL(shader.id), "!");
    location = static_cast<ShaderUniformLocation>(instance_texture_count++);
  }

  COMET_ASSERT(location != kInvalidShaderUniformLocation,
               "Location found is invalid for shader ",
               COMET_STRING_ID_LABEL(shader.id), "!");
  AddUniform(shader, uniform_descr, location);
}

ShaderUniformIndex ShaderHandler::HandleUniformIndex(
    Shader& shader, const ShaderUniformDescr& uniform_descr) const {
  ShaderUniformIndex* index{nullptr};

  if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len, "projection",
                      10)) {
    index = &shader.uniform_indices.projection;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len, "view",
                             4)) {
    index = &shader.uniform_indices.view;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             "ambientColor", 12)) {
    index = &shader.uniform_indices.ambient_color;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             "viewPos", 7)) {
    index = &shader.uniform_indices.view_pos;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             "diffuseColor", 12)) {
    index = &shader.uniform_indices.diffuse_color;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             "diffuseMap", 10)) {
    index = &shader.uniform_indices.diffuse_map;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             "specularMap", 11)) {
    index = &shader.uniform_indices.specular_map;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             "normalMap", 9)) {
    index = &shader.uniform_indices.normal_map;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             "shininess", 9)) {
    index = &shader.uniform_indices.shininess;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             "model", 5)) {
    index = &shader.uniform_indices.model;
  }

  COMET_ASSERT(index != nullptr, "Unable to find uniform location with name \"",
               uniform_descr.name, "\"!");
  COMET_ASSERT(*index == kInvalidShaderUniformIndex,
               "Uniform location with name \"", uniform_descr.name,
               "\" was already bound!");
  *index = static_cast<ShaderUniformIndex>(shader.uniforms.size());
  return *index;
}

void ShaderHandler::HandleDescriptorSetLayoutsGeneration(Shader& shader) const {
  for (u32 i{0}; i < shader.layout_bindings.count; ++i) {
    const auto info{init::GenerateDescriptorSetLayoutCreateInfo(
        shader.layout_bindings.list[i])};
    COMET_CHECK_VK(
        vkCreateDescriptorSetLayout(context_->GetDevice(), &info,
                                    VK_NULL_HANDLE, &shader.layout_handles[i]),
        "Could not creat descriptor set layout for shader ",
        COMET_STRING_ID_LABEL(shader.id), "!");
  }
}

void ShaderHandler::HandlePipelineGeneration(Shader& shader) const {
  const auto& device{context_->GetDevice()};

  PipelineDescr pipeline_descr{};
  pipeline_descr.id = shader.id;
  pipeline_descr.type = PipelineType::Graphics;
  pipeline_descr.render_pass = shader.render_pass;

  pipeline_descr.vertex_input_binding_description.binding = 0;
  pipeline_descr.vertex_input_binding_description.stride =
      shader.vertex_attribute_stride;
  pipeline_descr.vertex_input_binding_description.inputRate =
      VK_VERTEX_INPUT_RATE_VERTEX;
  pipeline_descr.vertex_attributes = &shader.vertex_attributes;
  pipeline_descr.descriptor_set_layout_handles = &shader.layout_handles;
  pipeline_descr.descriptor_set_layout_count = shader.layout_bindings.count;

  for (const auto& module : shader.modules) {
    pipeline_descr.shader_stages.push_back(
        init::GeneratePipelineShaderStageCreateInfo(module->type,
                                                    module->handle));
  }

  pipeline_descr.push_constant_ranges = &shader.push_constant_ranges;

  pipeline_descr.input_assembly_state =
      init::GeneratePipelineInputAssemblyStateCreateInfo(
          VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  pipeline_descr.rasterization_state =
      init::GeneratePipelineRasterizationStateCreateInfo(shader.is_wireframe,
                                                         shader.cull_mode);
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
          true, true, VK_COMPARE_OP_LESS  // New fragments should be less (lower
                                          // depth = closer).
      );

  shader.pipeline = pipeline_handler_->Generate(pipeline_descr);
}

void ShaderHandler::HandleDescriptorPoolGeneration(Shader& shader) const {
  constexpr VkDescriptorPoolSize pool_sizes[2]{
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4096}};

  shader.descriptor_pool_handle =
      shader.global_uniform_data.descriptor_pool_handle =
          GenerateDescriptorPool(
              context_->GetDevice(), 1024, pool_sizes, 2,
              VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
}

void ShaderHandler::HandleBufferGeneration(Shader& shader) const {
  const auto align{context_->GetDevice()
                       .GetProperties()
                       .limits.minUniformBufferOffsetAlignment};

  shader.global_ubo_data.ubo_stride = memory::AlignSize(
      shader.global_ubo_data.ubo_size, static_cast<memory::Alignment>(align));
  shader.instance_ubo_data.ubo_stride = memory::AlignSize(
      shader.instance_ubo_data.ubo_size, static_cast<memory::Alignment>(align));

#ifdef COMET_DEBUG
  const auto max_range{
      context_->GetDevice().GetProperties().limits.maxUniformBufferRange};
  COMET_ASSERT(shader.global_ubo_data.ubo_stride < max_range,
               "Shader global UBO data is too big!");
  COMET_ASSERT(shader.instance_ubo_data.ubo_stride < max_range,
               "Shader instance UBO data is too big!");
#endif  // COMET_DEBUG

  auto buffer_size{static_cast<VkDeviceSize>(
      shader.global_ubo_data.ubo_stride +
      shader.instance_ubo_data.ubo_stride * kMaxMaterialInstances)};

  shader.uniform_buffer = GenerateBuffer(
      context_->GetAllocatorHandle(), buffer_size,
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0,
      VK_SHARING_MODE_EXCLUSIVE, "uniform_buffer");

  const auto layout_count{descriptor_set_layout_handles_buffer_.size()};
  shader.global_uniform_data.descriptor_set_handles.resize(layout_count);

  for (u32 i{0}; i < layout_count; ++i) {
    descriptor_set_layout_handles_buffer_[i] =
        shader.layout_handles[kShaderDescriptorSetGlobalIndex];
  }

  AllocateShaderUniformData(context_->GetDevice(),
                            descriptor_set_layout_handles_buffer_,
                            shader.global_uniform_data);
}

void ShaderHandler::AddUniform(Shader& shader, const ShaderUniformDescr& descr,
                               ShaderUniformLocation location) const {
  COMET_ASSERT(shader.uniforms.size() + 1 <= kMaxShaderUniformCount,
               "Too many uniforms to be added to shader ",
               COMET_STRING_ID_LABEL(shader.id), "! Max uniform count is ",
               kMaxShaderUniformCount, ".");
  ShaderUniform uniform{};
  uniform.scope = descr.scope;
  uniform.type = descr.type;
  uniform.index = HandleUniformIndex(shader, descr);
  auto is_sampler{uniform.type == ShaderUniformType::Sampler};

  if (is_sampler) {
    uniform.location = location;
  } else {
    uniform.location = uniform.index;
  }

  auto size{GetUniformSize(uniform.type)};

  if (uniform.scope == ShaderUniformScope::Local) {
    uniform.offset =
        static_cast<ShaderOffset>(shader.push_constant_ranges.size());
    uniform.size = static_cast<ShaderUniformSize>(memory::AlignSize(
        size,
        static_cast<memory::Alignment>(GetStd430Alignment(uniform.type))));

    VkPushConstantRange range{};
    range.offset = uniform.offset;
    range.size = uniform.size;
    range.stageFlags =
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    shader.push_constant_ranges.push_back(range);
  } else {
    uniform.offset =
        is_sampler ? 0
        : uniform.scope == ShaderUniformScope::Global
            ? static_cast<ShaderOffset>(shader.global_ubo_data.ubo_size)
            : static_cast<ShaderOffset>(shader.instance_ubo_data.ubo_size);
    uniform.size = size;
  }

  shader.uniforms.push_back(uniform);

  if (!is_sampler) {
    if (uniform.scope == ShaderUniformScope::Global) {
      shader.global_ubo_data.ubo_size += uniform.size;
    } else if (uniform.scope == ShaderUniformScope::Instance) {
      shader.instance_ubo_data.ubo_size += uniform.size;
    }
  }
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet
