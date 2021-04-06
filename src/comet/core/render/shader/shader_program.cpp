// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "shader_program.hpp"

#include "utils/file_system.hpp"
#include "utils/logger.hpp"

#ifdef _WIN32
#include "debug_windows.hpp"
#endif  // _WIN32

namespace comet {
ShaderProgram::ShaderProgram(const char *vertex_shader_path,
                             const char *fragment_shader_path) {
  if (!filesystem::ReadFile(vertex_shader_path, &vertex_shader_code_)) {
    Logger::Get(kLoggerCometCoreRenderShaderShaderProgram)
        ->Error(
            "An error occurred while reading the vertex shader program file");

    return;
  }

  if (!filesystem::ReadFile(fragment_shader_path, &fragment_shader_code_)) {
    Logger::Get(kLoggerCometCoreRenderShaderShaderProgram)
        ->Error(
            "An error occurred while reading the fragment shader program file");

    return;
  }

  can_be_initialized_ = true;
}
void ShaderProgram::Initialize() {
  if (!can_be_initialized_) {
    Logger::Get(kLoggerCometCoreRenderShaderShaderProgram)
        ->Error(
            "An error occurred while creating the shader program. It can't be "
            "initialized");

    return;
  }

  CompileShader(&vertex_shader_id_, &vertex_shader_code_, GL_VERTEX_SHADER);

  CompileShader(&fragment_shader_id_, &fragment_shader_code_,
                GL_FRAGMENT_SHADER);

  id_ = glCreateProgram();
  glAttachShader(id_, vertex_shader_id_);
  glAttachShader(id_, fragment_shader_id_);
  glLinkProgram(id_);

  auto result = GL_FALSE;
  int info_log_len;

  // Check the program.
  glGetProgramiv(id_, GL_LINK_STATUS, &result);
  glGetProgramiv(id_, GL_INFO_LOG_LENGTH, &info_log_len);

  if (info_log_len > 0) {
    const auto logger = Logger::Get(kLoggerCometCoreRenderShaderShaderProgram);
    std::vector<GLchar> error_message(info_log_len + 1);

    glGetProgramInfoLog(id_, info_log_len, nullptr, &error_message[0]);

    logger->Error("Error while creating the shader program");
    logger->Error(std::string(error_message.data()));
  }

  glDetachShader(id_, vertex_shader_id_);
  glDetachShader(id_, fragment_shader_id_);

  glDeleteShader(vertex_shader_id_);
  glDeleteShader(fragment_shader_id_);
}

bool ShaderProgram::CompileShader(unsigned int *shader_id,
                                  const std::string *shader_code,
                                  GLenum shader_type) {
  // compile shader
  auto source_pointer = shader_code->c_str();

  *shader_id = glCreateShader(shader_type);
  glShaderSource(*shader_id, 1, &source_pointer, nullptr);
  glCompileShader(*shader_id);

  auto result = GL_FALSE;
  int info_log_len;

  // check shader
  glGetShaderiv(*shader_id, GL_COMPILE_STATUS, &result);
  glGetShaderiv(*shader_id, GL_INFO_LOG_LENGTH, &info_log_len);

  if (info_log_len > 0) {
    const auto logger = Logger::Get(kLoggerCometCoreRenderShaderShaderProgram);

    std::vector<char> error_message(info_log_len + 1);

    glGetShaderInfoLog(*shader_id, info_log_len, nullptr, &error_message[0]);

    logger->Error("Error while compiling shader for shader program");
    logger->Error(std::string(error_message.data()));

    return false;
  }

  if (result == GL_FALSE) {
    Logger::Get(kLoggerCometCoreRenderShaderShaderProgram)
        ->Error("Error while compiling shader for shader program");

    return false;
  }

  return true;
}

void ShaderProgram::Use() { glUseProgram(id_); }

void ShaderProgram::Destroy() { glDeleteProgram(id_); }

const unsigned int ShaderProgram::id() const noexcept { return id_; }

void ShaderProgram::SetFloat(const std::string &name, float v0) {
  glUniform1f(glGetUniformLocation(id_, name.c_str()), v0);
}

void ShaderProgram::SetFloat(const std::string &name, float v0, float v1) {
  glUniform2f(glGetUniformLocation(id_, name.c_str()), v0, v1);
}

void ShaderProgram::SetFloat(const std::string &name, float v0, float v1,
                             float v2) {
  glUniform3f(glGetUniformLocation(id_, name.c_str()), v0, v1, v2);
}

void ShaderProgram::SetFloat(const std::string &name, float v0, float v1,
                             float v2, float v3) {
  glUniform4f(glGetUniformLocation(id_, name.c_str()), v0, v1, v2, v3);
}

void ShaderProgram::SetInt(const std::string &name, int v0) {
  glUniform1i(glGetUniformLocation(id_, name.c_str()), v0);
}

void ShaderProgram::SetInt(const std::string &name, int v0, int v1) {
  glUniform2i(glGetUniformLocation(id_, name.c_str()), v0, v1);
}

void ShaderProgram::SetInt(const std::string &name, int v0, int v1, int v2) {
  glUniform3i(glGetUniformLocation(id_, name.c_str()), v0, v1, v2);
}

void ShaderProgram::SetInt(const std::string &name, int v0, int v1, int v2,
                           int v3) {
  glUniform4i(glGetUniformLocation(id_, name.c_str()), v0, v1, v2, v3);
}

void ShaderProgram::SetUnsignedInt(const std::string &name, unsigned int v0) {
  glUniform1ui(glGetUniformLocation(id_, name.c_str()), v0);
}

void ShaderProgram::SetUnsignedInt(const std::string &name, unsigned int v0,
                                   unsigned int v1) {
  glUniform2ui(glGetUniformLocation(id_, name.c_str()), v0, v1);
}

void ShaderProgram::SetUnsignedInt(const std::string &name, unsigned int v0,
                                   unsigned int v1, unsigned int v2) {
  glUniform3ui(glGetUniformLocation(id_, name.c_str()), v0, v1, v2);
}

void ShaderProgram::SetUnsignedInt(const std::string &name, unsigned int v0,
                                   unsigned int v1, unsigned int v2,
                                   unsigned int v3) {
  glUniform4ui(glGetUniformLocation(id_, name.c_str()), v0, v1, v2, v3);
}

void ShaderProgram::SetFloatArray1(const std::string &name, std::size_t count,
                                   const float *value) {
  glUniform1fv(glGetUniformLocation(id_, name.c_str()), count, value);
}

void ShaderProgram::SetFloatArray2(const std::string &name, std::size_t count,
                                   const float *value) {
  glUniform2fv(glGetUniformLocation(id_, name.c_str()), count, value);
}

void ShaderProgram::SetFloatArray3(const std::string &name, std::size_t count,
                                   const float *value) {
  glUniform3fv(glGetUniformLocation(id_, name.c_str()), count, value);
}

void ShaderProgram::SetFloatArray4(const std::string &name, std::size_t count,
                                   const float *value) {
  glUniform4fv(glGetUniformLocation(id_, name.c_str()), count, value);
}

void ShaderProgram::SetIntArray1(const std::string &name, std::size_t count,
                                 const int *value) {
  glUniform1iv(glGetUniformLocation(id_, name.c_str()), count, value);
}

void ShaderProgram::SetIntArray2(const std::string &name, std::size_t count,
                                 const int *value) {
  glUniform2iv(glGetUniformLocation(id_, name.c_str()), count, value);
}

void ShaderProgram::SetIntArray3(const std::string &name, std::size_t count,
                                 const int *value) {
  glUniform3iv(glGetUniformLocation(id_, name.c_str()), count, value);
}

void ShaderProgram::SetIntArray4(const std::string &name, std::size_t count,
                                 const int *value) {
  glUniform4iv(glGetUniformLocation(id_, name.c_str()), count, value);
}

void ShaderProgram::SetUnsignedIntArray1(const std::string &name,
                                         std::size_t count,
                                         const unsigned int *value) {
  glUniform1uiv(glGetUniformLocation(id_, name.c_str()), count, value);
}

void ShaderProgram::SetUnsignedIntArray2(const std::string &name,
                                         std::size_t count,
                                         const unsigned int *value) {
  glUniform2uiv(glGetUniformLocation(id_, name.c_str()), count, value);
}

void ShaderProgram::SetUnsignedIntArray3(const std::string &name,
                                         std::size_t count,
                                         const unsigned int *value) {
  glUniform3uiv(glGetUniformLocation(id_, name.c_str()), count, value);
}

void ShaderProgram::SetUnsignedIntArray4(const std::string &name,
                                         std::size_t count,
                                         const unsigned int *value) {
  glUniform4uiv(glGetUniformLocation(id_, name.c_str()), count, value);
}

void ShaderProgram::SetMatrix2(const std::string &name, const glm::mat2 &matrix,
                               bool transpose) {
  glUniformMatrix2fv(glGetUniformLocation(id_, name.c_str()), 1, transpose,
                     &matrix[0][0]);
}

void ShaderProgram::SetMatrix3(const std::string &name, const glm::mat3 &matrix,
                               bool transpose) {
  glUniformMatrix3fv(glGetUniformLocation(id_, name.c_str()), 1, transpose,
                     &matrix[0][0]);
}

void ShaderProgram::SetMatrix4(const std::string &name, const glm::mat4 &matrix,
                               bool transpose) {
  glUniformMatrix4fv(glGetUniformLocation(id_, name.c_str()), 1, transpose,
                     &matrix[0][0]);
}

void ShaderProgram::SetMatrix2x3(const std::string &name,
                                 const glm::mat2x3 &matrix, bool transpose) {
  glUniformMatrix2x3fv(glGetUniformLocation(id_, name.c_str()), 1, transpose,
                       &matrix[0][0]);
}

void ShaderProgram::SetMatrix3x2(const std::string &name,
                                 const glm::mat3x2 &matrix, bool transpose) {
  glUniformMatrix3x2fv(glGetUniformLocation(id_, name.c_str()), 1, transpose,
                       &matrix[0][0]);
}

void ShaderProgram::SetMatrix2x4(const std::string &name,
                                 const glm::mat2x4 &matrix, bool transpose) {
  glUniformMatrix2x4fv(glGetUniformLocation(id_, name.c_str()), 1, transpose,
                       &matrix[0][0]);
}

void ShaderProgram::SetMatrix4x2(const std::string &name,
                                 const glm::mat4x2 &matrix, bool transpose) {
  glUniformMatrix4x2fv(glGetUniformLocation(id_, name.c_str()), 1, transpose,
                       &matrix[0][0]);
}

void ShaderProgram::SetMatrix3x4(const std::string &name,
                                 const glm::mat3x4 &matrix, bool transpose) {
  glUniformMatrix3x4fv(glGetUniformLocation(id_, name.c_str()), 1, transpose,
                       &matrix[0][0]);
}

void ShaderProgram::SetMatrix4x3(const std::string &name,
                                 const glm::mat4x3 &matrix, bool transpose) {
  glUniformMatrix4x3fv(glGetUniformLocation(id_, name.c_str()), 1, transpose,
                       &matrix[0][0]);
}
}  // namespace comet
