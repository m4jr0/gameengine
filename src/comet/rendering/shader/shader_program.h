// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_SHADER_SHADER_PROGRAM_H_
#define COMET_COMET_RENDERING_SHADER_SHADER_PROGRAM_H_

#include "GL/glew.h"
#include "comet_precompile.h"
#include "glm/glm.hpp"

namespace comet {
namespace rendering {
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
  void SetFloat(const std::string&, float);
  void SetFloat(const std::string&, float, float);
  void SetFloat(const std::string&, float, float, float);
  void SetFloat(const std::string&, float, float, float, float);
  void SetInt(const std::string&, int);
  void SetInt(const std::string&, int, int);
  void SetInt(const std::string&, int, int, int);
  void SetInt(const std::string&, int, int, int, int);
  void SetUnsignedInt(const std::string&, unsigned int);
  void SetUnsignedInt(const std::string&, unsigned int, unsigned int);

  void SetUnsignedInt(const std::string&, unsigned int, unsigned int,
                      unsigned int);

  void SetUnsignedInt(const std::string&, unsigned int, unsigned int,
                      unsigned int, unsigned int);

  void SetFloatArray1(const std::string&, std::size_t, const float*);
  void SetFloatArray2(const std::string&, std::size_t, const float*);
  void SetFloatArray3(const std::string&, std::size_t, const float*);
  void SetFloatArray4(const std::string&, std::size_t, const float*);
  void SetIntArray1(const std::string&, std::size_t, const int*);
  void SetIntArray2(const std::string&, std::size_t, const int*);
  void SetIntArray3(const std::string&, std::size_t, const int*);
  void SetIntArray4(const std::string&, std::size_t, const int*);

  void SetUnsignedIntArray1(const std::string&, std::size_t,
                            const unsigned int*);

  void SetUnsignedIntArray2(const std::string&, std::size_t,
                            const unsigned int*);

  void SetUnsignedIntArray3(const std::string&, std::size_t,
                            const unsigned int*);

  void SetUnsignedIntArray4(const std::string&, std::size_t,
                            const unsigned int*);

  void SetMatrix2(const std::string&, const glm::mat2&, bool = GL_FALSE);
  void SetMatrix3(const std::string&, const glm::mat3&, bool = GL_FALSE);
  void SetMatrix4(const std::string&, const glm::mat4&, bool = GL_FALSE);

  void SetMatrix2x3(const std::string&, const glm::mat2x3&, bool = GL_FALSE);

  void SetMatrix3x2(const std::string&, const glm::mat3x2&, bool = GL_FALSE);

  void SetMatrix2x4(const std::string&, const glm::mat2x4&, bool = GL_FALSE);

  void SetMatrix4x2(const std::string&, const glm::mat4x2&, bool = GL_FALSE);

  void SetMatrix3x4(const std::string&, const glm::mat3x4&, bool = GL_FALSE);

  void SetMatrix4x3(const std::string&, const glm::mat4x3&, bool = GL_FALSE);

  const unsigned int GetId() const noexcept;

  template <class Archive>
  void Serialize(Archive& archive, const unsigned int file_version) {
    archive& vertex_shader_code_;
    archive& fragment_shader_code_;
  }

 private:
  bool CompileShader(unsigned int*, const std::string*, GLenum);

  unsigned int id_ = -1;
  unsigned int vertex_shader_id_ = -1;
  unsigned int fragment_shader_id_ = -1;
  std::string vertex_shader_code_;
  std::string fragment_shader_code_;
  bool can_be_initialized_ = false;
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_SHADER_SHADER_PROGRAM_H_
