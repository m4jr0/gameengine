// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_SHADER_OPENGL_SHADER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_SHADER_OPENGL_SHADER_H_

#include "comet_precompile.h"

#include "glad/glad.h"
#include "glm/glm.hpp"

namespace comet {
namespace rendering {
namespace gl {
class ShaderProgram {
 public:
  ShaderProgram(const char*, const char*);
  ShaderProgram(const ShaderProgram&);
  ShaderProgram(ShaderProgram&&) = default;
  ShaderProgram& operator=(const ShaderProgram&);
  ShaderProgram& operator=(ShaderProgram&&) = default;
  ~ShaderProgram() = default;

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
  void SetMatrix2(const std::string& name, const glm::mat2& matrix,
                  bool is_transpose = GL_FALSE);
  void SetMatrix3(const std::string& name, const glm::mat3& matrix,
                  bool is_transpose = GL_FALSE);
  void SetMatrix4(const std::string& name, const glm::mat4& matrix,
                  bool is_transpose = GL_FALSE);
  void SetMatrix2x3(const std::string& name, const glm::mat2x3& matrix,
                    bool is_transpose = GL_FALSE);
  void SetMatrix3x2(const std::string& name, const glm::mat3x2& matrix,
                    bool is_transpose = GL_FALSE);
  void SetMatrix2x4(const std::string& name, const glm::mat2x4& matrix,
                    bool is_transpose = GL_FALSE);
  void SetMatrix4x2(const std::string& name, const glm::mat4x2& matrix,
                    bool is_transpose = GL_FALSE);
  void SetMatrix3x4(const std::string& name, const glm::mat3x4& matrix,
                    bool is_transpose = GL_FALSE);
  void SetMatrix4x3(const std::string& name, const glm::mat4x3& matrix,
                    bool is_transpose = GL_FALSE);
  const u32 GetId() const noexcept;

 private:
  bool CompileShader(u32* shader_id, const std::string* shader_code,
                     GLenum shader_type);

  u32 id_{0};
  u32 vertex_shader_id_{0};
  u32 fragment_shader_id_{0};
  std::string vertex_shader_code_;
  std::string fragment_shader_code_;
  bool can_be_initialized_{false};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_SHADER_OPENGL_SHADER_H_
