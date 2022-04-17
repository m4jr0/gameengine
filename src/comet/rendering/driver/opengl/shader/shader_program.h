// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_SHADER_OPENGL_SHADER_PROGRAM_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_SHADER_OPENGL_SHADER_PROGRAM_H_

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
  virtual ~ShaderProgram() = default;

  void Initialize();
  void Use();
  void Destroy();
  void SetFloat(const std::string& name, float v0);
  void SetFloat(const std::string& name, float v0, float v1);
  void SetFloat(const std::string& name, float v0, float v1, float v2);
  void SetFloat(const std::string& name, float v0, float v1, float v2,
                float v3);
  void SetInt(const std::string& name, int v0);
  void SetInt(const std::string& name, int v0, int v1);
  void SetInt(const std::string& name, int v0, int v1, int v2);
  void SetInt(const std::string& name, int v0, int v1, int v2, int v3);
  void SetUnsignedInt(const std::string& name, unsigned int v0);
  void SetUnsignedInt(const std::string& name, unsigned int v0,
                      unsigned int v1);
  void SetUnsignedInt(const std::string& name, unsigned int v0, unsigned int v1,
                      unsigned int);
  void SetUnsignedInt(const std::string& name, unsigned int v0, unsigned int v1,
                      unsigned int v2, unsigned int v3);
  void SetFloatArray1(const std::string& name, std::size_t count,
                      const float* value);
  void SetFloatArray2(const std::string& name, std::size_t count,
                      const float* value);
  void SetFloatArray3(const std::string& name, std::size_t count,
                      const float* value);
  void SetFloatArray4(const std::string& name, std::size_t count,
                      const float* value);
  void SetIntArray1(const std::string& name, std::size_t count,
                    const int* value);
  void SetIntArray2(const std::string& name, std::size_t count,
                    const int* value);
  void SetIntArray3(const std::string& name, std::size_t count,
                    const int* value);
  void SetIntArray4(const std::string& name, std::size_t count,
                    const int* value);
  void SetUnsignedIntArray1(const std::string& name, std::size_t count,
                            const unsigned int* value);
  void SetUnsignedIntArray2(const std::string& name, std::size_t count,
                            const unsigned int* value);
  void SetUnsignedIntArray3(const std::string& name, std::size_t count,
                            const unsigned int* value);
  void SetUnsignedIntArray4(const std::string& name, std::size_t count,
                            const unsigned int* value);
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
  const unsigned int GetId() const noexcept;

  template <class Archive>
  void Serialize(Archive& archive, const unsigned int file_version) {
    archive& vertex_shader_code_;
    archive& fragment_shader_code_;
  }

 private:
  bool CompileShader(unsigned int* shader_id, const std::string* shader_code,
                     GLenum shader_type);

  unsigned int id_ = -1;
  unsigned int vertex_shader_id_ = -1;
  unsigned int fragment_shader_id_ = -1;
  std::string vertex_shader_code_;
  std::string fragment_shader_code_;
  bool can_be_initialized_ = false;
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_SHADER_OPENGL_SHADER_PROGRAM_H_
