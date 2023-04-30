// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_OPENGL_SHADER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_OPENGL_SHADER_H_

#include "comet_precompile.h"

#include "glad/glad.h"

#include "comet/math/matrix.h"
#include "comet/math/vector.h"
#include "comet/resource/shader_module_resource.h"

namespace comet {
namespace rendering {
namespace gl {
using ShaderHandle = u32;
constexpr auto kInvalidShaderHandle{0};

using ShaderModuleHandle = u32;
constexpr auto kInvalidShaderModuleHandle{0};

class ShaderProgram {
 public:
  ShaderProgram(const resource::ShaderModuleResource* vertex_shader_resource,
                const resource::ShaderModuleResource* fragment_shader_resource);
  ShaderProgram(const ShaderProgram&);
  ShaderProgram(ShaderProgram&&) = default;
  ShaderProgram& operator=(const ShaderProgram&) = default;
  ShaderProgram& operator=(ShaderProgram&&) = default;
  ~ShaderProgram();

  void Initialize();
  void Use();
  void Destroy();
  void SetFloat(const std::string& name, f32 v0);
  void SetFloat(const std::string& name, f32 v0, f32 v1);
  void SetFloat(const std::string& name, f32 v0, f32 v1, f32 v2);
  void SetFloat(const std::string& name, f32 v0, f32 v1, f32 v2, f32 v3);
  void SetInt(const std::string& name, s32 v0);
  void SetInt(const std::string& name, s32 v0, s32 v1);
  void SetInt(const std::string& name, s32 v0, s32 v1, s32 v2);
  void SetInt(const std::string& name, s32 v0, s32 v1, s32 v2, s32 v3);
  void SetUnsignedInt(const std::string& name, u32 v0);
  void SetUnsignedInt(const std::string& name, u32 v0, u32 v1);
  void SetUnsignedInt(const std::string& name, u32 v0, u32 v1, u32);
  void SetUnsignedInt(const std::string& name, u32 v0, u32 v1, u32 v2, u32 v3);
  void SetFloatArray1(const std::string& name, uindex count, const f32* value);
  void SetFloatArray2(const std::string& name, uindex count, const f32* value);
  void SetFloatArray3(const std::string& name, uindex count, const f32* value);
  void SetFloatArray4(const std::string& name, uindex count, const f32* value);
  void SetIntArray1(const std::string& name, uindex count, const s32* value);
  void SetIntArray2(const std::string& name, uindex count, const s32* value);
  void SetIntArray3(const std::string& name, uindex count, const s32* value);
  void SetIntArray4(const std::string& name, uindex count, const s32* value);
  void SetUnsignedIntArray1(const std::string& name, uindex count,
                            const u32* value);
  void SetUnsignedIntArray2(const std::string& name, uindex count,
                            const u32* value);
  void SetUnsignedIntArray3(const std::string& name, uindex count,
                            const u32* value);
  void SetUnsignedIntArray4(const std::string& name, uindex count,
                            const u32* value);
  void SetMatrix2(const std::string& name, const math::Mat2& matrix,
                  bool is_transpose = GL_FALSE);
  void SetMatrix3(const std::string& name, const math::Mat3& matrix,
                  bool is_transpose = GL_FALSE);
  void SetMatrix4(const std::string& name, const math::Mat4& matrix,
                  bool is_transpose = GL_FALSE);
  void SetMatrix2x3(const std::string& name, const math::Mat2x3& matrix,
                    bool is_transpose = GL_FALSE);
  void SetMatrix3x2(const std::string& name, const math::Mat3x2& matrix,
                    bool is_transpose = GL_FALSE);
  void SetMatrix2x4(const std::string& name, const math::Mat2x4& matrix,
                    bool is_transpose = GL_FALSE);
  void SetMatrix4x2(const std::string& name, const math::Mat4x2& matrix,
                    bool is_transpose = GL_FALSE);
  void SetMatrix3x4(const std::string& name, const math::Mat3x4& matrix,
                    bool is_transpose = GL_FALSE);
  void SetMatrix4x3(const std::string& name, const math::Mat4x3& matrix,
                    bool is_transpose = GL_FALSE);
  const ShaderHandle GetHandle() const noexcept;

 private:
  bool CompileShader(ShaderHandle* shader_id, const std::string* shader_code,
                     GLenum shader_type);

  bool is_initialized_{false};
  ShaderHandle handle_{kInvalidShaderHandle};
  ShaderModuleHandle vertex_shader_handle_{kInvalidShaderModuleHandle};
  ShaderModuleHandle fragment_shader_handle_{kInvalidShaderModuleHandle};
  std::string vertex_shader_code_{};
  std::string fragment_shader_code_{};
  bool can_be_initialized_{false};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_OPENGL_SHADER_H_
