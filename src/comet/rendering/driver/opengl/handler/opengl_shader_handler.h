// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_SHADER_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_SHADER_HANDLER_H_

#include <functional>

#include "glad/glad.h"

#include "comet/core/essentials.h"
#include "comet/math/matrix.h"
#include "comet/rendering/driver//opengl/data/opengl_material.h"
#include "comet/rendering/driver/opengl/data/opengl_frame.h"
#include "comet/rendering/driver/opengl/data/opengl_shader.h"
#include "comet/rendering/driver/opengl/handler/opengl_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_shader_module_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_texture_handler.h"
#include "comet/resource/shader_resource.h"

namespace comet {
namespace rendering {
namespace gl {
using ShaderUniformCallback =
    std::function<void(s32 location, const void* value)>;

struct ShaderPacket {
  FrameIndex frame_count{kInvalidFrameIndex};
  const math::Mat4* projection_matrix{nullptr};
  const math::Mat4* view_matrix{nullptr};
};

struct ShaderLocalPacket {
  const math::Mat4* position{nullptr};
};

struct ShaderHandlerDescr : HandlerDescr {
  ShaderModuleHandler* shader_module_handler{nullptr};
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

  void Shutdown() override;

  Shader* Generate(const ShaderDescr& descr);
  Shader* Get(ShaderId id);
  Shader* TryGet(ShaderId id);
  void Destroy(ShaderId id);
  void Destroy(Shader& shader);
  void Bind(Shader& shader);
  void BindGlobal(Shader& shader) const;
  void BindInstance(Shader& shader, MaterialInstanceId instance_id) const;
  void Reset();
  void UpdateGlobal(Shader& shader, const ShaderPacket& packet) const;
  void UpdateConstants(Shader& shader, const ShaderLocalPacket& packet);
  void SetUniform(Shader& shader, const ShaderUniform& uniform,
                  const void* value, bool is_update = true) const;
  void SetUniform(ShaderId id, const ShaderUniform& uniform, const void* value,
                  bool is_update = true);
  void SetUniform(Shader& shader, ShaderUniformLocation location,
                  const void* value, bool is_update = true) const;
  void SetUniform(ShaderId id, ShaderUniformLocation location,
                  const void* value, bool is_update = true);
  void BindMaterial(Material& material);
  void UnbindMaterial(Material& material);
  bool HasMaterial(const Material& material) const;
  MaterialInstance& GetInstance(Shader& shader, const Material& material);

 private:
  static GLenum GetCullMode(CullMode cull_mode);
  static ShaderUniformSize ShaderVariableTypeSize(ShaderVariableType type);
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
  void Destroy(Shader& shader, bool is_destroying_handler);
  u32 GetInstanceIndex(const Shader& shader, const Material& material) const;
  u32 GetInstanceIndex(const Shader& shader,
                       const MaterialInstanceId instance_id) const;

  void HandleUniformsGeneration(Shader& shader,
                                const resource::ShaderResource& resource) const;
  void HandleSamplerGeneration(Shader& shader,
                               const ShaderUniformDescr& uniform_descr,
                               u32& instance_texture_count) const;
  void HandleUniformCount(Shader& shader) const;
  ShaderUniformIndex HandleUniformIndex(
      Shader& shader, const ShaderUniformDescr& uniform_descr) const;
  void HandleBufferGeneration(Shader& shader) const;
  void AddUniform(Shader& shader, const ShaderUniformDescr& descr,
                  usize data_index) const;

  Shader* bound_shader_{nullptr};
  std::unordered_map<ShaderHandle, Shader> shaders_{};
  gid::BreedHandler instance_id_handler_{};
  ShaderModuleHandler* shader_module_handler_{nullptr};
  TextureHandler* texture_handler_{nullptr};
  inline static const std::unordered_map<ShaderVariableType,
                                         ShaderUniformCallback>
      kUniformCallbacks_{{ShaderVariableType::B32, SetS32},
                         {ShaderVariableType::S32, SetS32},
                         {ShaderVariableType::U32, SetU32},
                         {ShaderVariableType::F32, SetF32},
                         {ShaderVariableType::B32Vec2, SetS32Vec2},
                         {ShaderVariableType::B32Vec3, SetS32Vec3},
                         {ShaderVariableType::B32Vec4, SetS32Vec4},
                         {ShaderVariableType::S32Vec2, SetS32Vec2},
                         {ShaderVariableType::S32Vec3, SetS32Vec3},
                         {ShaderVariableType::S32Vec4, SetS32Vec4},
                         {ShaderVariableType::U32Vec2, SetU32Vec2},
                         {ShaderVariableType::U32Vec3, SetU32Vec3},
                         {ShaderVariableType::U32Vec4, SetU32Vec4},
                         {ShaderVariableType::Vec2, SetF32Vec2},
                         {ShaderVariableType::Vec3, SetF32Vec3},
                         {ShaderVariableType::Vec4, SetF32Vec4},
                         {ShaderVariableType::Mat2x2, SetMat2},
                         {ShaderVariableType::Mat2x3, SetMat2x3},
                         {ShaderVariableType::Mat2x4, SetMat2x4},
                         {ShaderVariableType::Mat3x2, SetMat3x2},
                         {ShaderVariableType::Mat3x3, SetMat3},
                         {ShaderVariableType::Mat3x4, SetMat3x4},
                         {ShaderVariableType::Mat4x2, SetMat4x2},
                         {ShaderVariableType::Mat4x3, SetMat4x3},
                         {ShaderVariableType::Mat4x4, SetMat4},
                         {ShaderVariableType::Sampler, SetS32}};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_SHADER_HANDLER_H_
