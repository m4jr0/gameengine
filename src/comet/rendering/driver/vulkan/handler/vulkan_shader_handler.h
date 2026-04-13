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
#include "comet/core/type/map.h"
#include "comet/rendering/driver/vulkan/data/vulkan_material.h"
#include "comet/rendering/driver/vulkan/data/vulkan_shader.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_descriptor_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_material_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_pipeline_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_render_pass_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_shader_module_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_texture_handler.h"
#include "comet/resource/resource.h"
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
  virtual ~ShaderHandler() = default;

  void Initialize() override;
  void Shutdown() override;

  Shader* Get(ShaderHandle shader_handle);
  const Shader* Get(ShaderHandle shader_handle) const;
  Shader* TryGet(ShaderHandle shader_handle);
  const Shader* TryGet(ShaderHandle shader_handle) const;

  Shader* GetOrGenerate(const ShaderDescr& descr);
  Shader* GetOrGenerate(resource::ResourceId shader_id,
                        RenderPassHandle render_pass_handle);

  void UnbindMaterialFromAllShaders(Material* material);
  void Destroy(ShaderHandle shader_handle);
  void Destroy(Shader* shader);

  ShaderBindingIndex GetBindingIndex(const Shader* shader, u32 set,
                                     u32 binding) const;
  void Bind(const Shader* shader, PipelineBindType pipeline_type) const;
  void BindInstance(Shader* shader, MaterialId material_id,
                    PipelineBindType pipeline_type);
  void BindInstance(Shader* shader, const Material* material,
                    PipelineBindType pipeline_type);

  void Reset();

  void UpdatePass(Shader* shader, const ShaderPassUpdate& update) const;
  void UpdateGlobals(Shader* shader, const ShaderGlobalUpdate& update) const;
  void UpdateInstance(Shader* shader, MaterialId material_id,
                      const ShaderInstanceUpdate& update);
  void UpdateInstance(Shader* shader, Material* material,
                      const ShaderInstanceUpdate& update);
  void PushConstants(Shader* shader,
                     const ShaderPushConstantsUpdate& update) const;

  void BindMaterial(Shader* shader, Material* material);
  void UnbindMaterial(Shader* shader, Material* material);
  bool HasMaterial(const Shader* shader, const Material* material) const;
  MaterialInstance& GetInstance(Shader* shader, const Material* material);
  const MaterialInstance* TryGetInstance(const Shader* shader,
                                         const Material* material) const;

 private:
  static bool HasVertexStage(const Shader* shader);
  static VkDescriptorType GetVkDescriptorType(ShaderBindingType type);

  static Alignment GetBindingFieldAlignment(ShaderMemoryLayout layout,
                                            ShaderVariableType type);
  static ShaderOffset AlignOffset(ShaderOffset offset, Alignment alignment);
  static ShaderFieldLayoutInfo GetFieldLayoutInfo(ShaderMemoryLayout layout,
                                                  ShaderVariableType type,
                                                  u32 array_count);
  static void PopulateVertexAttributes(
      ShaderVertexLayout layout,
      Array<VkVertexInputAttributeDescription>& attributes, u32& stride);
  static void PopulateSkinnedVertexAttributes(
      Array<VkVertexInputAttributeDescription>& attributes, u32& stride);
  static void PopulateDebugLineVertexAttributes(
      Array<VkVertexInputAttributeDescription>& attributes, u32& stride);

  Shader* Generate(const ShaderDescr& descr,
                   const resource::ShaderResource* shader_resource);
  void Destroy(Shader* shader, bool is_destroying_handler);

  void DestroyBindingRuntimeData(ShaderBindingRuntimeData& binding_data) const;
  void HandleShaderModulesGeneration(Shader* shader,
                                     const resource::ShaderResource* resource);
  void HandleAttributesGeneration(Shader* shader,
                                  const resource::ShaderResource* resource);
  void HandleBindingsGeneration(Shader* shader,
                                const resource::ShaderResource* resource);
  void HandlePushConstantBlocksGeneration(
      Shader* shader, const resource::ShaderResource* resource);

  void ComputeBindingLayout(ShaderBinding& binding) const;
  void ComputePushConstantBlockLayout(ShaderPushConstantBlock& block) const;

  void HandleDescriptorSetLayoutsGeneration(Shader* shader) const;
  void HandlePipelineGeneration(Shader* shader) const;
  void HandleGraphicsPipelineGeneration(
      Shader* shader, const PipelineLayout* pipeline_layout) const;
  void HandleComputePipelineGeneration(
      Shader* shader, const PipelineLayout* pipeline_layout) const;
  void HandleDescriptorPoolGeneration(Shader* shader) const;
  void HandleUboBufferGeneration(Shader* shader);

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

  void WriteBindingFieldToUbo(Buffer& buffer, ShaderOffset base_offset,
                              const ShaderBinding& binding,
                              ShaderFieldIndex field_index, const void* value,
                              usize value_size = 0) const;

  void WriteBindingFieldToMappedUbo(Buffer& buffer, ShaderOffset base_offset,
                                    const ShaderBinding& binding,
                                    ShaderFieldIndex field_index,
                                    const void* value, usize value_size) const;

  void UpdateInstanceUboDescriptors(Shader* shader, MaterialInstance& instance,
                                    u32 instance_index);

  ShaderBindingImageRuntimeData* FindBindingRuntimeData(
      ShaderBindingRuntimeData& binding_data,
      ShaderBindingIndex binding_index) const;

  u32 descriptor_set_layout_count_{0};
  memory::FiberFreeListAllocator general_allocator_{
      64, 256, memory::kEngineMemoryTagRendering};
  memory::FiberFreeListAllocator shader_instance_allocator_{
      sizeof(Shader), 1024, memory::kEngineMemoryTagRendering};
  Array<Shader*> shaders_{};
  Map<ShaderKey, ShaderHandle, ShaderKeyHashLogic> shader_keys_{};
  gid::BreedHandler shader_handle_handler_{};
  Shader* bound_shader_{nullptr};
  ShaderModuleHandler* shader_module_handler_{nullptr};
  PipelineHandler* pipeline_handler_{nullptr};
  MaterialHandler* material_handler_{nullptr};
  TextureHandler* texture_handler_{nullptr};
  DescriptorHandler* descriptor_handler_{nullptr};
  RenderPassHandler* render_pass_handler_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_SHADER_HANDLER_H_