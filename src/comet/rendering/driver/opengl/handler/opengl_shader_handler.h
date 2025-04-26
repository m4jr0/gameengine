// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_SHADER_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_SHADER_HANDLER_H_

#include <functional>

#include "glad/glad.h"

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/type/gid.h"
#include "comet/math/matrix.h"
#include "comet/rendering/driver//opengl/data/opengl_material.h"
#include "comet/rendering/driver/opengl/data/opengl_shader.h"
#include "comet/rendering/driver/opengl/handler/opengl_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_material_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_mesh_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_shader_module_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_texture_handler.h"
#include "comet/resource/shader_resource.h"

namespace comet {
namespace rendering {
namespace gl {
struct ShaderHandlerDescr : HandlerDescr {
  ShaderModuleHandler* shader_module_handler{nullptr};
  MaterialHandler* material_handler{nullptr};
  MeshHandler* mesh_handler{nullptr};
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

  Shader* Generate(const ShaderDescr& descr);
  Shader* Get(ShaderId shader_id);
  Shader* TryGet(ShaderId shader_id);
  void Bind(Shader* shader, ShaderBindType bind_type);
  void BindInstance(Shader* shader, MaterialId material_id);
  void BindInstance(Shader* shader, const Material* material);
  void Destroy(ShaderId shader_id);
  void Destroy(Shader* shader);
  void Reset();
  void UpdateGlobals(Shader* shader, const frame::FramePacket* packet) const;
  void UpdateGlobals(Shader* shader, FrameCount frame_count,
                     const ShaderGlobalsUpdate& update) const;
  void UpdateConstants(Shader* shader, const frame::FramePacket* packet) const;
  void UpdateConstants(Shader* shader,
                       const ShaderConstantsUpdate& update) const;
  void UpdateStorages(Shader* shader, const frame::FramePacket* packet) const;
  void UpdateStorages(Shader* shader, const ShaderStoragesUpdate& update) const;
  void UpdateInstance(Shader* shader, FrameCount frame_count,
                      MaterialId material_id);
  void UpdateInstance(Shader* shader, FrameCount frame_count,
                      Material* material);
  void SetUniform(Shader* shader, const ShaderUniform& uniform,
                  const void* value) const;
  void SetUniform(ShaderId id, const ShaderUniform& uniform, const void* value);
  void SetUniform(Shader* shader, ShaderUniformIndex index,
                  const void* value) const;
  void SetUniform(ShaderId id, ShaderUniformIndex index, const void* value);
  void SetConstant(const ShaderConstant& constant, const void* value) const;
  void SetConstant(ShaderId shader_id, ShaderConstantIndex index,
                   const void* value) const;
  void SetConstant(const Shader* shader, ShaderConstantIndex index,
                   const void* value) const;
  void BindMaterial(Material* material);
  void UnbindMaterial(Material* material);
  bool HasMaterial(const Material* material) const;
  MaterialInstance& GetInstance(Shader* shader, const Material* material);

 private:
  static ShaderUniformSize GetShaderVariableTypeSize(ShaderVariableType type);
  static GLenum GetGlCullMode(CullMode cull_mode);
  static GLenum GetGlPrimitiveTopology(PrimitiveTopology topology);
  static GLsizei GetGlAttributeSize(ShaderVertexAttributeType type);
  static GLint GetGlComponentCount(ShaderVertexAttributeType type);
  static GLenum GetGlVertexAttributeType(ShaderVertexAttributeType type);

  static void BindStorageBuffer(const Shader* shader,
                                ShaderStorageIndex storage_index,
                                GLuint buffer_handle);
  static void SetF32(s32 location, const void* value);
  static void SetS32(s32 location, const void* value);
  static void SetU32(s32 location, const void* value);
  static void SetF32Vec2(s32 location, const void* value);
  static void SetF32Vec3(s32 location, const void* value);
  static void SetF32Vec4(s32 location, const void* value);
  static void SetS32Vec2(s32 location, const void* value);
  static void SetS32Vec3(s32 location, const void* value);
  static void SetS32Vec4(s32 location, const void* value);
  static void SetU32Vec2(s32 location, const void* value);
  static void SetU32Vec3(s32 location, const void* value);
  static void SetU32Vec4(s32 location, const void* value);
  static void SetMat2(s32 location, const void* value);
  static void SetMat3(s32 location, const void* value);
  static void SetMat4(s32 location, const void* value);
  static void SetMat2x3(s32 location, const void* value);
  static void SetMat3x2(s32 location, const void* value);
  static void SetMat2x4(s32 location, const void* value);
  static void SetMat4x2(s32 location, const void* value);
  static void SetMat3x4(s32 location, const void* value);
  static void SetMat4x3(s32 location, const void* value);
  const Shader* Get(ShaderId shader_id) const;
  const Shader* TryGet(ShaderId shader_id) const;
  void Destroy(Shader* shader, bool is_destroying_handler);
  u32 GetInstanceIndex(const Shader* shader, const Material* material) const;
  u32 GetInstanceIndex(const Shader* shader,
                       const MaterialInstanceId instance_id) const;
  void HandleProgram(Shader* shader,
                     const resource::ShaderResource* shader_resource);
  void HandleProgramGeneration(Shader* shader) const;
  void HandleProgramCompilation(Shader* shader, ShaderBindType bind_type) const;
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
  void HandleUboBufferGeneration(Shader* shader) const;
  void AddUniform(Shader* shader, const ShaderUniformDescr& descr,
                  ShaderUniformLocation location) const;
  void AddConstant(Shader* shader, const ShaderConstantDescr& descr) const;
  void AddStorage(Shader* shader, const ShaderStorageDescr& descr) const;

  memory::FiberFreeListAllocator general_allocator_{
      64, 256, memory::kEngineMemoryTagRendering};
  memory::FiberFreeListAllocator shader_instance_allocator_{
      sizeof(Shader), 1024, memory::kEngineMemoryTagRendering};
  Map<ShaderId, Shader*> shaders_{};
  gid::BreedHandler instance_id_handler_{};
  ShaderHandle bound_shader_handle_{kInvalidShaderHandle};
  VertexAttributeHandle bound_vertex_attribute_handle_{
      kInvalidVertexAttributeHandle};
  ShaderModuleHandler* shader_module_handler_{nullptr};
  MaterialHandler* material_handler_{nullptr};
  MeshHandler* mesh_handler_{nullptr};
  TextureHandler* texture_handler_{nullptr};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_SHADER_HANDLER_H_
