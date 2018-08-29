// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_RENDER_SHADER_SHADER_PROGRAM_HPP_
#define KOMA_CORE_RENDER_SHADER_SHADER_PROGRAM_HPP_

#define LOGGER_KOMA_CORE_RENDER_SHADER_SHADER_PROGRAM "koma_core_render"

#include <GL/glew.h>
#include <string>

namespace koma {
class ShaderProgram {
 public:
  ShaderProgram(const char *, const char *);

  void Use();
  void Delete();

  void SetFloat(const std::string &, float);
  void SetFloat(const std::string &, float, float);
  void SetFloat(const std::string &, float, float, float);
  void SetFloat(const std::string &, float, float, float, float);
  void SetInt(const std::string &, int);
  void SetInt(const std::string &, int, int);
  void SetInt(const std::string &, int, int, int);
  void SetInt(const std::string &, int, int, int, int);
  void SetUnsignedInt(const std::string &, unsigned int);
  void SetUnsignedInt(const std::string &, unsigned int, unsigned int);

  void SetUnsignedInt(const std::string &, unsigned int, unsigned int,
                      unsigned int);

  void SetUnsignedInt(const std::string &, unsigned int, unsigned int,
                      unsigned int, unsigned int);

  void SetFloatArray1(const std::string &, std::size_t, const float *);
  void SetFloatArray2(const std::string &, std::size_t, const float *);
  void SetFloatArray3(const std::string &, std::size_t, const float *);
  void SetFloatArray4(const std::string &, std::size_t, const float *);
  void SetIntArray1(const std::string &, std::size_t, const int *);
  void SetIntArray2(const std::string &, std::size_t, const int *);
  void SetIntArray3(const std::string &, std::size_t, const int *);
  void SetIntArray4(const std::string &, std::size_t, const int *);

  void SetUnsignedIntArray1(const std::string &, std::size_t,
                            const unsigned int *);

  void SetUnsignedIntArray2(const std::string &, std::size_t,
                            const unsigned int *);

  void SetUnsignedIntArray3(const std::string &, std::size_t,
                            const unsigned int *);

  void SetUnsignedIntArray4(const std::string &, std::size_t,
                            const unsigned int *);

  void SetMatrix2fv(const std::string &, std::size_t, bool, const float *);
  void SetMatrix3fv(const std::string &, std::size_t, bool, const float *);
  void SetMatrix4fv(const std::string &, std::size_t, bool, const float *);
  void SetMatrix2x3fv(const std::string &, std::size_t, bool, const float *);
  void SetMatrix3x2fv(const std::string &, std::size_t, bool, const float *);
  void SetMatrix2x4fv(const std::string &, std::size_t, bool, const float *);
  void SetMatrix4x2fv(const std::string &, std::size_t, bool, const float *);
  void SetMatrix3x4fv(const std::string &, std::size_t, bool, const float *);
  void SetMatrix4x3fv(const std::string &, std::size_t, bool, const float *);

  const unsigned int id() const noexcept;

 private:
   unsigned int id_ = -1;
   unsigned int vertex_shader_id_ = -1;
   unsigned int fragment_shader_id_ = -1;
   bool CompileShader(unsigned int *, std::string *, GLenum);
};
};  // namespace koma

#endif  // KOMA_CORE_RENDER_SHADER_SHADER_PROGRAM_HPP_
