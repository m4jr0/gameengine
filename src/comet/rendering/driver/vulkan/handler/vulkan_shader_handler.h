// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_SHADER_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_SHADER_HANDLER_H_

#include "vulkan/vulkan.h"

#include "comet/core/essentials.h"
#include "comet/math/matrix.h"
#include "comet/rendering/driver/vulkan/data/vulkan_material.h"
#include "comet/rendering/driver/vulkan/data/vulkan_shader.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_pipeline_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_shader_module_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_texture_handler.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_initializer_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"
#include "comet/resource/shader_resource.h"

namespace comet {
namespace rendering {
namespace vk {
struct ShaderPacket {
  const math::Mat4* projection_matrix{nullptr};
  const math::Mat4* view_matrix{nullptr};
};

struct ShaderLocalPacket {
  const math::Mat4* position{nullptr};
};

struct ShaderHandlerDescr : HandlerDescr {
  ShaderModuleHandler* shader_module_handler{nullptr};
  PipelineHandler* pipeline_handler{nullptr};
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
  void Destroy(ShaderId shader_id);
  void Destroy(Shader& shader);
  void Bind(Shader& shader);
  void BindGlobal(Shader& shader) const;
  void BindInstance(Shader& shader, MaterialInstanceId instance_id) const;
  void Reset();
  void UpdateGlobal(Shader& shader, const ShaderPacket& packet) const;
  void UpdateLocal(Shader& shader, const ShaderLocalPacket& packet);
  void SetUniform(Shader& shader, const ShaderUniform& uniform,
                  const void* value) const;
  void SetUniform(ShaderId id, const ShaderUniform& uniform, const void* value);
  void SetUniform(Shader& shader, ShaderUniformLocation index,
                  const void* value) const;
  void SetUniform(ShaderId id, ShaderUniformLocation index, const void* value);
  void BindMaterial(Material& material);
  void UnbindMaterial(Material& material);
  bool HasMaterial(const Material& material) const;
  MaterialInstance& GetInstance(Shader& shader, const Material& material);

 private:
  static ShaderVertexAttributeSize GetVertexAttributeSize(
      ShaderVertexAttributeType type);
  static ShaderUniformSize GetUniformSize(ShaderUniformType type);
  static VkFormat GetVkFormat(ShaderVertexAttributeType type);

  void Destroy(Shader& shader, bool is_destroying_handler);
  u32 GetInstanceIndex(const Shader& shader, const Material& material) const;
  u32 GetInstanceIndex(const Shader& shader,
                       const MaterialInstanceId instance_id) const;
  void HandleShaderModulesGeneration(
      Shader& shader, const resource::ShaderResource& resource) const;
  void HandleAttributesGeneration(
      Shader& shader, const resource::ShaderResource& resource) const;
  void HandleBindingsGeneration(Shader& shader) const;
  void HandleUniformsGeneration(Shader& shader,
                                const resource::ShaderResource& resource) const;
  void HandleSamplerGeneration(Shader& shader,
                               const ShaderUniformDescr& uniform_descr,
                               u32& instance_texture_count) const;
  ShaderUniformIndex HandleUniformIndex(
      Shader& shader, const ShaderUniformDescr& uniform_descr) const;
  void HandleDescriptorSetLayoutsGeneration(Shader& shader) const;
  void HandlePipelineGeneration(Shader& shader) const;
  void HandleDescriptorPoolGeneration(Shader& shader) const;
  void HandleBufferGeneration(Shader& shader) const;
  void AddUniform(Shader& shader, const ShaderUniformDescr& descr,
                  ShaderUniformLocation location) const;

  std::unordered_map<ShaderId, Shader> shaders_{};
  gid::BreedHandler instance_id_handler_{};
  mutable std::vector<VkDescriptorSetLayout>
      descriptor_set_layout_handles_buffer_{};
  Shader* bound_shader_{nullptr};
  ShaderModuleHandler* shader_module_handler_{nullptr};
  PipelineHandler* pipeline_handler_{nullptr};
  TextureHandler* texture_handler_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_SHADER_HANDLER_H_