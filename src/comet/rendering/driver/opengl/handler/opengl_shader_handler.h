// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_SHADER_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_SHADER_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/type/array.h"
#include "comet/core/type/gid.h"
#include "comet/core/type/map.h"
#include "comet/rendering/driver/opengl/data/opengl_material.h"
#include "comet/rendering/driver/opengl/data/opengl_mesh.h"
#include "comet/rendering/driver/opengl/data/opengl_shader.h"
#include "comet/rendering/driver/opengl/handler/opengl_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_material_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_shader_module_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_texture_handler.h"
#include "comet/resource/resource.h"
#include "comet/resource/shader_resource.h"

namespace comet {
namespace rendering {
namespace gl {
struct ShaderHandlerDescr : HandlerDescr {
  ShaderModuleHandler* shader_module_handler{nullptr};
  MaterialHandler* material_handler{nullptr};
  TextureHandler* texture_handler{nullptr};
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
  Shader* GetOrGenerate(resource::ResourceId shader_id);

  void UnbindMaterialFromAllShaders(Material* material);
  void Destroy(ShaderHandle shader_handle);
  void Destroy(Shader* shader);

  ShaderBindingIndex GetBindingIndex(const Shader* shader, u32 set,
                                     u32 binding) const;
  void Bind(const Shader* shader, ShaderBindType bind_type) const;
  void BindInstance(Shader* shader, MaterialId material_id);
  void BindInstance(Shader* shader, const Material* material);
  void BindVertexSource(Shader* shader, const ShaderVertexSource& source) const;

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
  static GLenum GetGlCullMode(CullMode cull_mode);
  static GLenum GetGlPrimitiveTopology(PrimitiveTopology topology);
  static GLenum GetGlCompareOp(CompareOp compare_op);

  static Alignment GetBindingFieldAlignment(ShaderMemoryLayout layout,
                                            ShaderVariableType type);
  static ShaderOffset AlignOffset(ShaderOffset offset, Alignment alignment);
  static ShaderFieldLayoutInfo GetFieldLayoutInfo(ShaderMemoryLayout layout,
                                                  ShaderVariableType type,
                                                  u32 array_count);

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
  static void BindImageBinding(const ShaderBinding& binding,
                               const ShaderImageDescriptor* descriptors,
                               u32 descriptor_count);

  Shader* Generate(const resource::ShaderResource* shader_resource);
  void Destroy(Shader* shader, bool is_destroying_handler);

  void DestroyBindingRuntimeData(ShaderBindingRuntimeData& binding_data) const;

  void HandleShaderModulesGeneration(Shader* shader,
                                     const resource::ShaderResource* resource);
  void HandleProgramGeneration(Shader* shader) const;
  void HandleProgramCompilation(Shader* shader, ShaderBindType bind_type) const;
  void HandleAttributesGeneration(Shader* shader,
                                  const resource::ShaderResource* resource);
  void HandleBindingsGeneration(Shader* shader,
                                const resource::ShaderResource* resource);
  void HandlePushConstantBlocksGeneration(
      Shader* shader, const resource::ShaderResource* resource);

  void ComputeBindingLayout(ShaderBinding& binding) const;
  void ComputePushConstantBlockLayout(ShaderPushConstantBlock& block) const;
  void ComputeUniformBlockLayout(ShaderPushConstantBlock& block) const;
  void HandleUboBufferGeneration(Shader* shader) const;

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

  void WriteBindingFieldToUbo(Shader* shader, ShaderOffset base_offset,
                              const ShaderBinding& binding,
                              ShaderFieldIndex field_index, const void* value,
                              usize value_size = 0) const;

  void UpdateInstanceUboBindings(Shader* shader, MaterialInstance& instance,
                                 u32 instance_index);

  ShaderBindingImageRuntimeData* FindBindingRuntimeData(
      ShaderBindingRuntimeData& binding_data,
      ShaderBindingIndex binding_index) const;

  ShaderOffset GetPushConstantBaseOffset(const Shader* shader) const;

  memory::FiberFreeListAllocator general_allocator_{
      64, 256, memory::kEngineMemoryTagRendering};
  memory::FiberFreeListAllocator shader_instance_allocator_{
      sizeof(Shader), 1024, memory::kEngineMemoryTagRendering};

  Array<Shader*> shaders_{};
  Map<resource::ResourceId, ShaderHandle> shader_ids_{};
  gid::BreedHandler shader_handle_handler_{};

  mutable const Shader* bound_shader_{nullptr};
  mutable VertexAttributeHandle bound_vertex_attribute_handle_{
      kInvalidVertexAttributeHandle};
  mutable GLuint bound_array_buffer_handle_{kInvalidStorageHandle};
  mutable GLuint bound_element_array_buffer_handle_{kInvalidStorageHandle};
  mutable GLuint bound_program_handle_{kInvalidShaderHandle};

  ShaderModuleHandler* shader_module_handler_{nullptr};
  MaterialHandler* material_handler_{nullptr};
  TextureHandler* texture_handler_{nullptr};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_SHADER_HANDLER_H_