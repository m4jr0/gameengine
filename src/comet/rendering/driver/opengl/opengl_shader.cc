// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "opengl_shader.h"

#include "comet/core/engine.h"
#include "comet/resource/shader_module_resource.h"
#include "comet/utils/file_system.h"

namespace comet {
namespace rendering {
namespace gl {
ShaderProgram::ShaderProgram(const schar* vertex_shader_path,
                             const schar* fragment_shader_path) {
  const auto* vertex_shader{
      Engine::Get().GetResourceManager().Load<resource::ShaderModuleResource>(
          vertex_shader_path)};

  vertex_shader_code_ =
      std::string{reinterpret_cast<const schar*>(vertex_shader->data.data()),
                  vertex_shader->data.size()};

  const auto* fragment_shader{
      Engine::Get().GetResourceManager().Load<resource::ShaderModuleResource>(
          fragment_shader_path)};

  fragment_shader_code_ =
      std::string{reinterpret_cast<const schar*>(fragment_shader->data.data()),
                  fragment_shader->data.size()};

  can_be_initialized_ = true;
}

ShaderProgram::ShaderProgram(const schar* vertex_shader_path,
                             const std::string& fragment_shader_path)
    : ShaderProgram{vertex_shader_path, fragment_shader_path.c_str()} {}

ShaderProgram::ShaderProgram(const std::string& vertex_shader_path,
                             const schar* fragment_shader_path)
    : ShaderProgram{vertex_shader_path.c_str(), fragment_shader_path} {}

ShaderProgram::ShaderProgram(const std::string& vertex_shader_path,
                             const std::string& fragment_shader_path)
    : ShaderProgram{vertex_shader_path.c_str(), fragment_shader_path.c_str()} {}

ShaderProgram::ShaderProgram(const ShaderProgram& other)
    : vertex_shader_code_{other.vertex_shader_code_},
      fragment_shader_code_{other.fragment_shader_code_},
      can_be_initialized_{!vertex_shader_code_.empty() &&
                          !fragment_shader_code_.empty()} {}

ShaderProgram::~ShaderProgram() {
  COMET_ASSERT(
      !is_initialized_,
      "Destructor called for shader program, but it is still initialized!");
}

void ShaderProgram::Initialize() {
  COMET_ASSERT(!is_initialized_,
               "Tried to initialize shader program, but it is already done!");

  COMET_ASSERT(
      can_be_initialized_,
      "An error occurred while creating the shader program. It can't be "
      "initialized");

  CompileShader(&vertex_shader_handle_, &vertex_shader_code_, GL_VERTEX_SHADER);

  CompileShader(&fragment_shader_handle_, &fragment_shader_code_,
                GL_FRAGMENT_SHADER);

  handle_ = glCreateProgram();
  glAttachShader(handle_, vertex_shader_handle_);
  glAttachShader(handle_, fragment_shader_handle_);
  glLinkProgram(handle_);

  auto result{GL_FALSE};
  GLsizei info_log_len{0};

  // Check the program.
  glGetProgramiv(handle_, GL_LINK_STATUS, &result);
  glGetProgramiv(handle_, GL_INFO_LOG_LENGTH, &info_log_len);

  if (info_log_len > 0) {
    std::vector<GLchar> error_message(info_log_len + 1);
    glGetProgramInfoLog(handle_, info_log_len, nullptr, &error_message[0]);
    COMET_ASSERT(false, "Error while creating the shader program: ",
                 error_message.data());
  }

  glDetachShader(handle_, vertex_shader_handle_);
  glDetachShader(handle_, fragment_shader_handle_);

  glDeleteShader(vertex_shader_handle_);
  glDeleteShader(fragment_shader_handle_);
  is_initialized_ = true;
}

bool ShaderProgram::CompileShader(ShaderHandle* shader_handle,
                                  const std::string* shader_code,
                                  GLenum shader_type) {
  // Compile the shader.
  const auto source_pointer{shader_code->c_str()};

  *shader_handle = glCreateShader(shader_type);
  glShaderSource(*shader_handle, 1, &source_pointer, nullptr);
  glCompileShader(*shader_handle);

  auto result{GL_FALSE};
  GLsizei info_log_len{0};

  // Check the shader.
  glGetShaderiv(*shader_handle, GL_COMPILE_STATUS, &result);
  glGetShaderiv(*shader_handle, GL_INFO_LOG_LENGTH, &info_log_len);

  if (info_log_len > 0) {
    std::vector<schar> error_message(info_log_len + 1);
    glGetShaderInfoLog(*shader_handle, info_log_len, nullptr,
                       &error_message[0]);
    COMET_ASSERT(false, "Error while creating the shader program: ",
                 error_message.data());
  }

  if (result == GL_FALSE) {
    COMET_LOG_RENDERING_ERROR(
        "Error while compiling shader for shader program");

    return false;
  }

  return true;
}

void ShaderProgram::Use() { glUseProgram(handle_); }

void ShaderProgram::Destroy() {
  glDeleteProgram(handle_);
  handle_ = kInvalidShaderHandle;
  vertex_shader_handle_ = kInvalidShaderModuleHandle;
  fragment_shader_handle_ = kInvalidShaderModuleHandle;
  vertex_shader_code_.clear();
  fragment_shader_code_.clear();
  can_be_initialized_ = false;
  is_initialized_ = false;
}

const ShaderHandle ShaderProgram::GetHandle() const noexcept { return handle_; }

void ShaderProgram::SetFloat(const std::string& name, f32 v0) {
  glUniform1f(glGetUniformLocation(handle_, name.c_str()), v0);
}

void ShaderProgram::SetFloat(const std::string& name, f32 v0, f32 v1) {
  glUniform2f(glGetUniformLocation(handle_, name.c_str()), v0, v1);
}

void ShaderProgram::SetFloat(const std::string& name, f32 v0, f32 v1, f32 v2) {
  glUniform3f(glGetUniformLocation(handle_, name.c_str()), v0, v1, v2);
}

void ShaderProgram::SetFloat(const std::string& name, f32 v0, f32 v1, f32 v2,
                             f32 v3) {
  glUniform4f(glGetUniformLocation(handle_, name.c_str()), v0, v1, v2, v3);
}

void ShaderProgram::SetInt(const std::string& name, s32 v0) {
  glUniform1i(glGetUniformLocation(handle_, name.c_str()), v0);
}

void ShaderProgram::SetInt(const std::string& name, s32 v0, s32 v1) {
  glUniform2i(glGetUniformLocation(handle_, name.c_str()), v0, v1);
}

void ShaderProgram::SetInt(const std::string& name, s32 v0, s32 v1, s32 v2) {
  glUniform3i(glGetUniformLocation(handle_, name.c_str()), v0, v1, v2);
}

void ShaderProgram::SetInt(const std::string& name, s32 v0, s32 v1, s32 v2,
                           s32 v3) {
  glUniform4i(glGetUniformLocation(handle_, name.c_str()), v0, v1, v2, v3);
}

void ShaderProgram::SetUnsignedInt(const std::string& name, u32 v0) {
  glUniform1ui(glGetUniformLocation(handle_, name.c_str()), v0);
}

void ShaderProgram::SetUnsignedInt(const std::string& name, u32 v0, u32 v1) {
  glUniform2ui(glGetUniformLocation(handle_, name.c_str()), v0, v1);
}

void ShaderProgram::SetUnsignedInt(const std::string& name, u32 v0, u32 v1,
                                   u32 v2) {
  glUniform3ui(glGetUniformLocation(handle_, name.c_str()), v0, v1, v2);
}

void ShaderProgram::SetUnsignedInt(const std::string& name, u32 v0, u32 v1,
                                   u32 v2, u32 v3) {
  glUniform4ui(glGetUniformLocation(handle_, name.c_str()), v0, v1, v2, v3);
}

void ShaderProgram::SetFloatArray1(const std::string& name, uindex count,
                                   const f32* value) {
  glUniform1fv(glGetUniformLocation(handle_, name.c_str()), count, value);
}

void ShaderProgram::SetFloatArray2(const std::string& name, uindex count,
                                   const f32* value) {
  glUniform2fv(glGetUniformLocation(handle_, name.c_str()), count, value);
}

void ShaderProgram::SetFloatArray3(const std::string& name, uindex count,
                                   const f32* value) {
  glUniform3fv(glGetUniformLocation(handle_, name.c_str()), count, value);
}

void ShaderProgram::SetFloatArray4(const std::string& name, uindex count,
                                   const f32* value) {
  glUniform4fv(glGetUniformLocation(handle_, name.c_str()), count, value);
}

void ShaderProgram::SetIntArray1(const std::string& name, uindex count,
                                 const s32* value) {
  glUniform1iv(glGetUniformLocation(handle_, name.c_str()), count, value);
}

void ShaderProgram::SetIntArray2(const std::string& name, uindex count,
                                 const s32* value) {
  glUniform2iv(glGetUniformLocation(handle_, name.c_str()), count, value);
}

void ShaderProgram::SetIntArray3(const std::string& name, uindex count,
                                 const s32* value) {
  glUniform3iv(glGetUniformLocation(handle_, name.c_str()), count, value);
}

void ShaderProgram::SetIntArray4(const std::string& name, uindex count,
                                 const s32* value) {
  glUniform4iv(glGetUniformLocation(handle_, name.c_str()), count, value);
}

void ShaderProgram::SetUnsignedIntArray1(const std::string& name, uindex count,
                                         const u32* value) {
  glUniform1uiv(glGetUniformLocation(handle_, name.c_str()), count, value);
}

void ShaderProgram::SetUnsignedIntArray2(const std::string& name, uindex count,
                                         const u32* value) {
  glUniform2uiv(glGetUniformLocation(handle_, name.c_str()), count, value);
}

void ShaderProgram::SetUnsignedIntArray3(const std::string& name, uindex count,
                                         const u32* value) {
  glUniform3uiv(glGetUniformLocation(handle_, name.c_str()), count, value);
}

void ShaderProgram::SetUnsignedIntArray4(const std::string& name, uindex count,
                                         const u32* value) {
  glUniform4uiv(glGetUniformLocation(handle_, name.c_str()), count, value);
}

void ShaderProgram::SetMatrix2(const std::string& name, const glm::mat2& matrix,
                               bool is_transpose) {
  glUniformMatrix2fv(glGetUniformLocation(handle_, name.c_str()), 1,
                     is_transpose, &matrix[0][0]);
}

void ShaderProgram::SetMatrix3(const std::string& name, const glm::mat3& matrix,
                               bool is_transpose) {
  glUniformMatrix3fv(glGetUniformLocation(handle_, name.c_str()), 1,
                     is_transpose, &matrix[0][0]);
}

void ShaderProgram::SetMatrix4(const std::string& name, const glm::mat4& matrix,
                               bool is_transpose) {
  glUniformMatrix4fv(glGetUniformLocation(handle_, name.c_str()), 1,
                     is_transpose, &matrix[0][0]);
}

void ShaderProgram::SetMatrix2x3(const std::string& name,
                                 const glm::mat2x3& matrix, bool is_transpose) {
  glUniformMatrix2x3fv(glGetUniformLocation(handle_, name.c_str()), 1,
                       is_transpose, &matrix[0][0]);
}

void ShaderProgram::SetMatrix3x2(const std::string& name,
                                 const glm::mat3x2& matrix, bool is_transpose) {
  glUniformMatrix3x2fv(glGetUniformLocation(handle_, name.c_str()), 1,
                       is_transpose, &matrix[0][0]);
}

void ShaderProgram::SetMatrix2x4(const std::string& name,
                                 const glm::mat2x4& matrix, bool is_transpose) {
  glUniformMatrix2x4fv(glGetUniformLocation(handle_, name.c_str()), 1,
                       is_transpose, &matrix[0][0]);
}

void ShaderProgram::SetMatrix4x2(const std::string& name,
                                 const glm::mat4x2& matrix, bool is_transpose) {
  glUniformMatrix4x2fv(glGetUniformLocation(handle_, name.c_str()), 1,
                       is_transpose, &matrix[0][0]);
}

void ShaderProgram::SetMatrix3x4(const std::string& name,
                                 const glm::mat3x4& matrix, bool is_transpose) {
  glUniformMatrix3x4fv(glGetUniformLocation(handle_, name.c_str()), 1,
                       is_transpose, &matrix[0][0]);
}

void ShaderProgram::SetMatrix4x3(const std::string& name,
                                 const glm::mat4x3& matrix, bool is_transpose) {
  glUniformMatrix4x3fv(glGetUniformLocation(handle_, name.c_str()), 1,
                       is_transpose, &matrix[0][0]);
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet
