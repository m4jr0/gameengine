// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Allow debugging memory leaks.
#include "../../../debug.hpp"

#include "shader_loader.hpp"

#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

// Some of this code was directly inspired from there:
// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-5-a-textured-cube/

namespace koma {
bool ReadFile(std::string *buffer, std::string file_path) {
  std::ifstream input_stream(file_path, std::ios::in);

  if (!input_stream.is_open()) {
    Logger::Get(LOGGER_KOMA_CORE_RENDERING)->Error(
      "Unable to open ", file_path
    );

    return false;
  }

  std::stringstream string_stream;

  string_stream << input_stream.rdbuf();
  *buffer = string_stream.str();

  input_stream.close();

  return true;
}

bool CompileShader(GLuint *shader_id, std::string *shader_code) {
  // compile shader
  char const *source_pointer = shader_code->c_str();
  glShaderSource(*shader_id, 1, &source_pointer, nullptr);
  glCompileShader(*shader_id);

  GLint result = GL_FALSE;
  int info_log_len;

  // check shader
  glGetShaderiv(*shader_id, GL_COMPILE_STATUS, &result);
  glGetShaderiv(*shader_id, GL_INFO_LOG_LENGTH, &info_log_len);

  if (info_log_len > 0) {
    auto logger = Logger::Get(LOGGER_KOMA_CORE_RENDERING);

    std::vector<char> error_message(info_log_len + 1);

    glGetShaderInfoLog(
      *shader_id, info_log_len, nullptr, &error_message[0]
    );

    logger->Error(
      "Error while compiling shader program with id ", *shader_id
    );

    logger->Error(
      std::string(error_message.data())
    );

    return false;
  }

  return true;
}

GLuint LoadShaders(const char *vertex_file_path,
                   const char *fragment_file_path) {
  GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

  std::string vertex_shader_code;
  std::string fragment_shader_code;

  if (!ReadFile(&vertex_shader_code, vertex_file_path) ||
    !ReadFile(&fragment_shader_code, fragment_file_path)) {
    return 0;
  }

  CompileShader(&vertex_shader_id, &vertex_shader_code);
  CompileShader(&fragment_shader_id, &fragment_shader_code);

  GLuint program_id;

  if ((program_id =
    CreateShaderProgram(vertex_shader_id, fragment_shader_id)) < 0) {
    return 0;
  }

  glDetachShader(program_id, vertex_shader_id);
  glDetachShader(program_id, fragment_shader_id);

  glDeleteShader(vertex_shader_id);
  glDeleteShader(fragment_shader_id);

  return program_id;
}
};  // namespace koma
