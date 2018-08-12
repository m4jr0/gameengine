// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_RENDERING_SHADER_LOADER_HPP_
#define KOMA_CORE_RENDERING_SHADER_LOADER_HPP_

#define LOGGER_KOMA_CORE_RENDERING "koma_core_rendering"

#include <GL/glew.h>
#include <string>

// Some of this code was directly inspired from there:
// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-5-a-textured-cube/

namespace koma {
  bool ReadFile(std::string *, std::string);
  bool CompileShader(GLuint *, std::string *);

  template<typename... Targs>
  GLuint CreateShaderProgram(Targs... shader_ids) {
    // create the program
    GLuint program_id = glCreateProgram();
    GLint result = GL_FALSE;
    int info_log_len;

    for (GLuint shader_id : { shader_ids... }) {
      glAttachShader(program_id, shader_id);
    }

    glLinkProgram(program_id);

    // check the program
    glGetProgramiv(program_id, GL_LINK_STATUS, &result);
    glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_log_len);

    if (info_log_len > 0) {
      std::vector<GLchar> error_message(info_log_len + 1);
      glGetProgramInfoLog(program_id, info_log_len, nullptr, &error_message[0]);

      std::cerr << "Error while creating shader programs" << std::endl;
      std::cerr << std::string(error_message.data()) << std::endl;

      return -1;
    }

    return program_id;
  }

  GLuint LoadShaders(const char *, const char *);
};  // namespace koma

#endif  // KOMA_CORE_RENDERING_SHADER_LOADER_HPP_
