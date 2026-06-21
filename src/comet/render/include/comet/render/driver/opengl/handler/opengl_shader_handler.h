// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_SHADER_HANDLER_H_
#define COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_SHADER_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/runtime/memory/allocator/free_list_allocator.h"
#include "comet/core/container/array.h"
#include "comet/runtime/shared_instance_registry.h"
#include "comet/render/driver/opengl/handler/opengl_handler.h"
#include "comet/render/driver/opengl/handler/opengl_material_handler.h"
#include "comet/render/driver/opengl/handler/opengl_sampler_handler.h"
#include "comet/render/driver/opengl/handler/opengl_shader_module_handler.h"
#include "comet/render/driver/opengl/handler/opengl_texture_handler.h"
#include "comet/render/driver/opengl/type/opengl_material.h"
#include "comet/render/driver/opengl/type/opengl_mesh.h"
#include "comet/render/driver/opengl/type/opengl_shader.h"
#include "comet/render/render_handle.h"
#include "comet/data/render/pipeline.h"
#include "comet/data/render/shader.h"
#include "comet/data/resource/shader/shader_resource.h"

namespace comet {
namespace render {
namespace gl {
struct ShaderHandlerDescr : HandlerDescr {
  ShaderModuleHandler* shader_module_handler{nullptr};
  MaterialHandler* material_handler{nullptr};
  const TextureHandler* texture_handler{nullptr};
  const SamplerHandler* sampler_handler{nullptr};
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
                             PipelineBindType bind_type);
  void Destroy(ShaderHandle handle);

  void Bind(ShaderHandle handle);
  void BindInstance(ShaderHandle handle, MaterialHandle material_handle);
  void BindInstance(ShaderHandle handle, const Material* material);
  void BindVertexSource(ShaderHandle handle, const ShaderVertexSource& source);

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
  GLenum GetTopology(ShaderHandle handle) const;

  void Reset();

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

  static void PopulateVertexAttributes(ShaderVertexLayout layout,
                                       Array<VertexAttribute>& attributes,
                                       VertexAttributeStride& stride);
  static void PopulateSkinnedVertexAttributes(
      Array<VertexAttribute>& attributes, VertexAttributeStride& stride);
  static void PopulateDebugLineVertexAttributes(
      Array<VertexAttribute>& attributes, VertexAttributeStride& stride);

  static void BindBufferBinding(const ShaderBinding& binding,
                                GLuint buffer_handle, usize buffer_size,
                                usize buffer_offset = 0);
  void BindImageBinding(const ShaderBinding& binding,
                        const ShaderImageDescriptor* descriptors,
                        u32 descriptor_count) const;

  bool HasMaterial(const Shader* shader, const Material* material) const;
  void BindMaterial(Shader* shader, const Material* material);
  void UnbindMaterial(Shader* shader, const Material* material);

  void HandleShaderModulesGeneration(Shader* shader,
                                     const resource::ShaderResource* resource);
  void HandleProgramGeneration(Shader* shader) const;
  void HandleProgramCompilation(Shader* shader) const;
  void HandleAttributesGeneration(Shader* shader,
                                  const resource::ShaderResource* resource);
  void HandleBindingsGeneration(Shader* shader,
                                const resource::ShaderResource* resource);
  void HandlePushConstantBlocksGeneration(
      Shader* shader, const resource::ShaderResource* resource);
  void HandleUboBufferGeneration(Shader* shader) const;

  void ComputeBindingLayout(ShaderBinding& binding) const;
  void ComputePushConstantBlockLayout(ShaderPushConstantBlock& block) const;
  void ComputeUniformBlockLayout(ShaderPushConstantBlock& block) const;

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

  void UpdateBindingBuffer(const ShaderBinding& binding, GLuint buffer_handle,
                           usize buffer_size, usize buffer_offset = 0) const;
  void UpdateBindingImages(const ShaderBinding& binding,
                           const ShaderImageDescriptor* descriptors,
                           u32 descriptor_count) const;
  void UpdateInstanceUboBindings(Shader* shader, MaterialInstance& instance,
                                 u32 instance_index);
  void WriteBindingFieldToUbo(Shader* shader, ShaderOffset base_offset,
                              const ShaderBinding& binding,
                              ShaderFieldIndex field_index, const void* value,
                              usize value_size = 0) const;

  ShaderBindingImageRuntimeData* FindBindingRuntimeData(
      ShaderBindingRuntimeData& binding_data,
      ShaderBindingIndex binding_index) const;
  void DestroyBindingRuntimeData(ShaderBindingRuntimeData& binding_data) const;

  ShaderOffset GetPushConstantBaseOffset(const Shader* shader) const;

  const MaterialHandler* GetMaterialHandler() const;

  memory::PlatformAllocator cache_allocator_{kEngineMemoryTagRender};
  memory::FiberFreeListAllocator general_allocator_{
      64, 256, kEngineMemoryTagRender};
  memory::FiberFreeListAllocator shader_instance_allocator_{
      sizeof(Shader), 1024, kEngineMemoryTagRender};
  SharedInstanceRegistry<ShaderKey, ShaderTag, Shader> shaders_{};
  mutable const Shader* bound_shader_{nullptr};
  mutable GlNativeVertexAttributeHandle bound_vertex_attribute_native_handle_{
      kInvalidGlNativeVertexAttributeHandle};
  mutable GlNativeStorageHandle bound_array_buffer_native_handle_{
      kInvalidGlNativeStorageHandle};
  mutable GlNativeStorageHandle bound_element_array_buffer_native_handle_{
      kInvalidGlNativeStorageHandle};
  mutable GlNativeProgramHandle bound_program_native_handle_{
      kInvalidGlNativeProgramHandle};
  ShaderModuleHandler* shader_module_handler_{nullptr};
  MaterialHandler* material_handler_{nullptr};
  const TextureHandler* texture_handler_{nullptr};
  const SamplerHandler* sampler_handler_{nullptr};
};
}  // namespace gl
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_SHADER_HANDLER_H_