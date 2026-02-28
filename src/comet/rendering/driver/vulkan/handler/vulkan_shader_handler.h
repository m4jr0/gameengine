// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_SHADER_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_SHADER_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/array.h"
#include "comet/core/type/map.h"
#include "comet/rendering/driver/vulkan/data/vulkan_material.h"
#include "comet/rendering/driver/vulkan/data/vulkan_shader.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_descriptor_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_material_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_pipeline_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_shader_module_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_texture_handler.h"
#include "comet/resource/shader_resource.h"

namespace comet {
namespace rendering {
namespace vk {
struct ShaderHandlerDescr : HandlerDescr {
  ShaderModuleHandler* shader_module_handler{nullptr};
  PipelineHandler* pipeline_handler{nullptr};
  MaterialHandler* material_handler{nullptr};
  TextureHandler* texture_handler{nullptr};
  DescriptorHandler* descriptor_handler{nullptr};
};

class ShaderHandler : public Handler {
 public:
  ShaderHandler() = delete;
  explicit ShaderHandler(const ShaderHandlerDescr& descr);
  ShaderHandler(const ShaderHandler&) = delete;
  ShaderHandler(ShaderHandler&&) = delete;
  ShaderHandler& operator=(const ShaderHandler&) = delete;
  ShaderHandler& operator=(ShaderHandler&&) = delete;
  virtual ~ShaderHandler() = default;

  void Initialize() override;
  void Shutdown() override;

  Shader* Generate(const ShaderDescr& descr);
  Shader* Get(ShaderId shader_id);
  Shader* TryGet(ShaderId shader_id);
  void Bind(const Shader* shader, PipelineBindType pipeline_type) const;
  void BindInstance(Shader* shader, MaterialId material_id,
                    PipelineBindType pipeline_type);
  void BindInstance(Shader* shader, const Material* material,
                    PipelineBindType pipeline_type);
  void Destroy(ShaderId shader_id);
  void Destroy(Shader* shader);
  void Reset();
  void UpdateGlobals(Shader* shader, const frame::FramePacket* packet) const;
  void UpdateGlobals(Shader* shader, const ShaderGlobalsUpdate& update) const;
  void UpdateConstants(Shader* shader, const frame::FramePacket* packet) const;
  void UpdateConstants(Shader* shader,
                       const ShaderConstantsUpdate& update) const;
  void UpdateStorages(Shader* shader, const frame::FramePacket* packet) const;
  void UpdateStorages(Shader* shader, const ShaderStoragesUpdate& update) const;
  void UpdateInstance(Shader* shader, MaterialId material_id);
  void UpdateInstance(Shader* shader, Material* material);
  void SetUniform(Shader* shader, const ShaderUniform& uniform,
                  const void* value) const;
  void SetUniform(ShaderId id, const ShaderUniform& uniform, const void* value);
  void SetUniform(Shader* shader, ShaderUniformIndex index,
                  const void* value) const;
  void SetUniform(ShaderId id, ShaderUniformIndex index, const void* value);
  void SetConstant(Shader* shader, const ShaderConstant& constant,
                   const void* value) const;
  void SetConstant(ShaderId id, const ShaderConstant& constant,
                   const void* value);
  void SetConstant(Shader* shader, ShaderConstantIndex index,
                   const void* value) const;
  void SetConstant(ShaderId id, ShaderConstantIndex index, const void* value);
  void BindMaterial(Material* material);
  void UnbindMaterial(Material* material);
  bool HasMaterial(const Material* material) const;
  MaterialInstance& GetInstance(Shader* shader, const Material* material);

 private:
  static ShaderVertexAttributeSize GetVertexAttributeSize(
      ShaderVertexAttributeType type);
  static ShaderUniformSize GetShaderVariableTypeSize(ShaderVariableType type);
  static VkFormat GetVkFormat(ShaderVertexAttributeType type);

  static void BindStorageBuffer(
      const Shader* shader, VkDescriptorSet set_handle,
      ShaderStorageIndex storage_index, VkBuffer buffer_handle,
      VkDeviceSize buffer_size, Array<VkDescriptorBufferInfo>& buffer_info,
      Array<VkDescriptorSetLayoutBinding>& bindings,
      Array<VkWriteDescriptorSet>& write_descriptor_sets);

  void Destroy(Shader* shader, bool is_destroying_handler);
  u32 GetInstanceIndex(const Shader* shader, const Material* material) const;
  u32 GetInstanceIndex(const Shader* shader,
                       const MaterialInstanceId instance_id) const;
  void HandleShaderModulesGeneration(
      Shader* shader, const resource::ShaderResource* resource) const;
  void HandleAttributesGeneration(
      Shader* shader, const resource::ShaderResource* resource) const;
  void HandleUniformsGeneration(Shader* shader,
                                const resource::ShaderResource* resource) const;
  void HandleConstantsGeneration(
      Shader* shader, const resource::ShaderResource* resource) const;
  void HandleStorageGeneration(Shader* shader,
                               const resource::ShaderResource* resource) const;
  void HandleBindingsGeneration(Shader* shader) const;
  void HandleSamplerGeneration(Shader* shader,
                               const ShaderUniformDescr& uniform_descr,
                               u32& instance_texture_count) const;
  ShaderUniformIndex HandleUniformIndex(
      Shader* shader, const ShaderUniformDescr& uniform_descr) const;
  ShaderConstantIndex HandleConstantIndex(
      Shader* shader, const ShaderConstantDescr& constant_descr) const;
  void HandleStorageIndexAndBinding(Shader* shader,
                                    const ShaderStorageDescr& storage_descr,
                                    ShaderStorage& storage) const;
  void HandleDescriptorSetLayoutsGeneration(Shader* shader) const;
  void HandlePipelineGeneration(Shader* shader) const;
  void HandleGraphicsPipelineGeneration(
      Shader* shader, const PipelineLayout* pipeline_layout) const;
  void HandleComputePipelineGeneration(
      Shader* shader, const PipelineLayout* pipeline_layout) const;
  void HandleDescriptorPoolGeneration(Shader* shader) const;
  void HandleUboBufferGeneration(Shader* shader);
  void AddUniform(Shader* shader, const ShaderUniformDescr& descr,
                  ShaderUniformLocation location) const;
  void AddConstant(Shader* shader, const ShaderConstantDescr& descr) const;
  void AddStorage(Shader* shader, const ShaderStorageDescr& descr) const;

  u32 descriptor_set_layout_count_{0};
  memory::FiberFreeListAllocator general_allocator_{
      64, 256, memory::kEngineMemoryTagRendering};
  memory::FiberFreeListAllocator shader_instance_allocator_{
      sizeof(Shader), 1024, memory::kEngineMemoryTagRendering};
  Map<ShaderId, Shader*> shaders_{};
  gid::BreedHandler instance_id_handler_{};
  Shader* bound_shader_{nullptr};
  ShaderModuleHandler* shader_module_handler_{nullptr};
  PipelineHandler* pipeline_handler_{nullptr};
  MaterialHandler* material_handler_{nullptr};
  TextureHandler* texture_handler_{nullptr};
  DescriptorHandler* descriptor_handler_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_SHADER_HANDLER_H_