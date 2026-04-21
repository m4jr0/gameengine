// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_SHADER_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_SHADER_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/array.h"
#include "comet/core/type/shared_instance_registry.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_descriptor_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_material_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_pipeline_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_render_pass_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_sampler_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_shader_module_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_texture_handler.h"
#include "comet/rendering/driver/vulkan/type/vulkan_material_type.h"
#include "comet/rendering/driver/vulkan/type/vulkan_shader_type.h"
#include "comet/rendering/rendering_handle.h"
#include "comet/resource/shader/shader_resource.h"

namespace comet {
namespace rendering {
namespace vk {
struct ShaderHandlerDescr : HandlerDescr {
  ShaderModuleHandler* shader_module_handler{nullptr};
  PipelineHandler* pipeline_handler{nullptr};
  MaterialHandler* material_handler{nullptr};
  const TextureHandler* texture_handler{nullptr};
  const SamplerHandler* sampler_handler{nullptr};
  DescriptorHandler* descriptor_handler{nullptr};
  RenderPassHandler* render_pass_handler{nullptr};
};

class ShaderHandler : public Handler {
 public:
  ShaderHandler() = delete;
  explicit ShaderHandler(const ShaderHandlerDescr& descr);
  ShaderHandler(const ShaderHandler&) = delete;
  ShaderHandler(ShaderHandler&&) = delete;
  ShaderHandler& operator=(const ShaderHandler&) = delete;
  ShaderHandler& operator=(ShaderHandler&&) = delete;
  ~ShaderHandler() override = default;

  ShaderHandle GetOrGenerate(const ShaderDescr& descr);
  ShaderHandle GetOrGenerate(resource::ShaderResourceId shader_resource_id,
                             RenderPassHandle render_pass_handle);
  void Destroy(ShaderHandle handle);

  void Bind(ShaderHandle handle, PipelineBindType pipeline_type) const;
  void BindInstance(ShaderHandle handle, MaterialHandle material_handle,
                    PipelineBindType pipeline_type);
  void BindInstance(ShaderHandle handle, const Material* material,
                    PipelineBindType pipeline_type);

  void UpdateGlobals(ShaderHandle handle, const ShaderGlobalUpdate& update);
  void UpdatePass(ShaderHandle handle, const ShaderPassUpdate& update);
  void UpdateInstance(ShaderHandle handle, MaterialHandle material_handle,
                      const ShaderInstanceUpdate& update);
  void UpdateInstance(ShaderHandle handle, const Material* material,
                      const ShaderInstanceUpdate& update);
  void PushConstants(ShaderHandle handle,
                     const ShaderPushConstantsUpdate& update);

  void BindMaterial(ShaderHandle handle, MaterialHandle material_handle);
  void BindMaterial(ShaderHandle handle, const Material* material);
  void UnbindMaterial(ShaderHandle handle, MaterialHandle material_handle);
  void UnbindMaterial(ShaderHandle handle, const Material* material);
  void UnbindMaterialFromAllShaders(const Material* material);

  bool HasMaterial(ShaderHandle handle, MaterialHandle material_handle) const;
  bool HasMaterial(ShaderHandle handle, const Material* material) const;

  ShaderBindingIndex GetBindingIndex(ShaderHandle handle, u32 set,
                                     u32 binding) const;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  Shader* Get(ShaderHandle handle);
  const Shader* Get(ShaderHandle handle) const;
  MaterialInstance& GetInstance(Shader* shader, const Material* material);
  const MaterialInstance* TryGetInstance(const Shader* shader,
                                         const Material* material) const;

  Shader* GenerateShader(const ShaderDescr& descr,
                         const resource::ShaderResource* shader_resource);
  void DestroyShader(Shader* shader);

  static bool HasVertexStage(const Shader& shader,
                             const ShaderModuleHandler& shader_module_handler);

  static void PopulateVertexAttributes(
      ShaderVertexLayout layout,
      Array<VkVertexInputAttributeDescription>& attributes, u32& stride);
  static void PopulateSkinnedVertexAttributes(
      Array<VkVertexInputAttributeDescription>& attributes, u32& stride);
  static void PopulateDebugLineVertexAttributes(
      Array<VkVertexInputAttributeDescription>& attributes, u32& stride);

  bool HasMaterial(const Shader* shader, const Material* material) const;
  void BindMaterial(Shader* shader, const Material* material);
  void UnbindMaterial(Shader* shader, const Material* material);

  void HandleShaderModulesGeneration(Shader* shader,
                                     const resource::ShaderResource* resource);
  void HandleAttributesGeneration(Shader* shader,
                                  const resource::ShaderResource* resource);
  void HandleBindingsGeneration(Shader* shader,
                                const resource::ShaderResource* resource);
  void HandlePushConstantBlocksGeneration(
      Shader* shader, const resource::ShaderResource* resource);
  void HandleDescriptorSetLayoutsGeneration(Shader* shader) const;
  void HandlePipelineGeneration(Shader* shader) const;
  void HandleGraphicsPipelineGeneration(
      Shader* shader, PipelineLayoutHandle pipeline_layout_handle) const;
  void HandleComputePipelineGeneration(
      Shader* shader, PipelineLayoutHandle pipeline_layout_handle) const;
  void HandleDescriptorPoolGeneration(Shader* shader) const;
  void HandleUboBufferGeneration(Shader* shader);

  void ComputeBindingLayout(ShaderBinding& binding) const;
  void ComputePushConstantBlockLayout(ShaderPushConstantBlock& block) const;

  void AllocateGlobalDescriptorSets(Shader* shader);
  void AllocateStorageDescriptorSets(Shader* shader);
  void AllocateMaterialDescriptorSets(Shader* shader,
                                      MaterialInstance& instance);

  void CollectMaterialTextureMaps(const Material* material,
                                  const ShaderBinding& binding,
                                  Array<const TextureMap*>& texture_maps) const;
  void CollectMaterialImageDescriptors(
      const Material* material, const ShaderBinding& binding,
      Array<ShaderImageDescriptor>& descriptors) const;

  void UpdateDescriptorSetImages(VkDescriptorSet set_handle,
                                 const ShaderBinding& binding,
                                 const ShaderImageDescriptor* descriptors,
                                 u32 descriptor_count) const;
  void UpdateDescriptorSetBuffer(VkDescriptorSet set_handle,
                                 const ShaderBinding& binding,
                                 VkBuffer buffer_handle, VkDeviceSize range,
                                 VkDeviceSize offset = 0) const;
  void UpdateInstanceUboDescriptors(Shader* shader, MaterialInstance& instance,
                                    u32 instance_index);
  void WriteBindingFieldToUbo(Buffer& buffer, ShaderOffset base_offset,
                              const ShaderBinding& binding,
                              ShaderFieldIndex field_index, const void* value,
                              usize value_size = 0) const;
  void WriteBindingFieldToMappedUbo(Buffer& buffer, ShaderOffset base_offset,
                                    const ShaderBinding& binding,
                                    ShaderFieldIndex field_index,
                                    const void* value, usize value_size) const;

  ShaderBindingImageRuntimeData* FindBindingRuntimeData(
      ShaderBindingRuntimeData& binding_data,
      ShaderBindingIndex binding_index) const;
  void DestroyBindingRuntimeData(ShaderBindingRuntimeData& binding_data) const;

  const MaterialHandler* GetMaterialHandler() const;

  u32 descriptor_set_layout_count_{0};

  memory::PlatformAllocator cache_allocator_{memory::kEngineMemoryTagRendering};
  memory::FiberFreeListAllocator general_allocator_{
      64, 256, memory::kEngineMemoryTagRendering};
  memory::FiberFreeListAllocator shader_instance_allocator_{
      sizeof(Shader), 1024, memory::kEngineMemoryTagRendering};

  SharedInstanceRegistry<ShaderKey, ShaderTag, Shader> shaders_{};

  ShaderModuleHandler* shader_module_handler_{nullptr};
  PipelineHandler* pipeline_handler_{nullptr};
  MaterialHandler* material_handler_{nullptr};
  const TextureHandler* texture_handler_{nullptr};
  const SamplerHandler* sampler_handler_{nullptr};
  DescriptorHandler* descriptor_handler_{nullptr};
  RenderPassHandler* render_pass_handler_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_SHADER_HANDLER_H_