// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Allow debugging memory leaks.
#include "debug.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "core/locator/locator.hpp"
#include "core/render/shader/shader_loader.hpp"
#include "core/render/texture/texture_loader.hpp"

// TODO(m4jr0): Remove this file (and its uses) when a proper game object
// handling will be added.
namespace koma {
GLuint vertex_array_id;
GLuint program_id = -1;
GLuint matrix_id = -1;

static const GLfloat g_vertex_buffer_data[] = {
  -1.0f,-1.0f,-1.0f,
  -1.0f,-1.0f, 1.0f,
  -1.0f, 1.0f, 1.0f,
  1.0f, 1.0f,-1.0f,
  -1.0f,-1.0f,-1.0f,
  -1.0f, 1.0f,-1.0f,
  1.0f,-1.0f, 1.0f,
  -1.0f,-1.0f,-1.0f,
  1.0f,-1.0f,-1.0f,
  1.0f, 1.0f,-1.0f,
  1.0f,-1.0f,-1.0f,
  -1.0f,-1.0f,-1.0f,
  -1.0f,-1.0f,-1.0f,
  -1.0f, 1.0f, 1.0f,
  -1.0f, 1.0f,-1.0f,
  1.0f,-1.0f, 1.0f,
  -1.0f,-1.0f, 1.0f,
  -1.0f,-1.0f,-1.0f,
  -1.0f, 1.0f, 1.0f,
  -1.0f,-1.0f, 1.0f,
  1.0f,-1.0f, 1.0f,
  1.0f, 1.0f, 1.0f,
  1.0f,-1.0f,-1.0f,
  1.0f, 1.0f,-1.0f,
  1.0f,-1.0f,-1.0f,
  1.0f, 1.0f, 1.0f,
  1.0f,-1.0f, 1.0f,
  1.0f, 1.0f, 1.0f,
  1.0f, 1.0f,-1.0f,
  -1.0f, 1.0f,-1.0f,
  1.0f, 1.0f, 1.0f,
  -1.0f, 1.0f,-1.0f,
  -1.0f, 1.0f, 1.0f,
  1.0f, 1.0f, 1.0f,
  -1.0f, 1.0f, 1.0f,
  1.0f,-1.0f, 1.0f
};

static const GLfloat g_uv_buffer_data[] = {
  0.000059f, 1.0f - 0.000004f,
  0.000103f, 1.0f - 0.336048f,
  0.335973f, 1.0f - 0.335903f,
  1.000023f, 1.0f - 0.000013f,
  0.667979f, 1.0f - 0.335851f,
  0.999958f, 1.0f - 0.336064f,
  0.667979f, 1.0f - 0.335851f,
  0.336024f, 1.0f - 0.671877f,
  0.667969f, 1.0f - 0.671889f,
  1.000023f, 1.0f - 0.000013f,
  0.668104f, 1.0f - 0.000013f,
  0.667979f, 1.0f - 0.335851f,
  0.000059f, 1.0f - 0.000004f,
  0.335973f, 1.0f - 0.335903f,
  0.336098f, 1.0f - 0.000071f,
  0.667979f, 1.0f - 0.335851f,
  0.335973f, 1.0f - 0.335903f,
  0.336024f, 1.0f - 0.671877f,
  1.000004f, 1.0f - 0.671847f,
  0.999958f, 1.0f - 0.336064f,
  0.667979f, 1.0f - 0.335851f,
  0.668104f, 1.0f - 0.000013f,
  0.335973f, 1.0f - 0.335903f,
  0.667979f, 1.0f - 0.335851f,
  0.335973f, 1.0f - 0.335903f,
  0.668104f, 1.0f - 0.000013f,
  0.336098f, 1.0f - 0.000071f,
  0.000103f, 1.0f - 0.336048f,
  0.000004f, 1.0f - 0.671870f,
  0.336024f, 1.0f - 0.671877f,
  0.000103f, 1.0f - 0.336048f,
  0.336024f, 1.0f - 0.671877f,
  0.335973f, 1.0f - 0.335903f,
  0.667969f, 1.0f - 0.671889f,
  1.000004f, 1.0f - 0.671847f,
  0.667979f, 1.0f - 0.335851f
};

GLuint texture = -1;
GLuint texture_id = -1;

GLuint vertex_buffer = -1;
GLuint uv_buffer = -1;

void InitializeTmp(GLuint width, GLuint height) {
  glGenVertexArrays(1, &vertex_array_id);
  glBindVertexArray(vertex_array_id);

  program_id = LoadShaders(
    "tmp/TextureVertexShader.vertexshader",
    "tmp/TextureFragmentShader.fragmentshader"
  );

  matrix_id = glGetUniformLocation(program_id, "mvp");

  texture = load_dds("tmp/texture_BMP_DXT3_1.DDS");

  texture_id = glGetUniformLocation(program_id, "texture_sampler");

  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

  glBufferData(
    GL_ARRAY_BUFFER,
    sizeof(g_vertex_buffer_data),
    g_vertex_buffer_data,
    GL_STATIC_DRAW
  );

  glGenBuffers(1, &uv_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);

  glBufferData(
    GL_ARRAY_BUFFER,
    sizeof(g_uv_buffer_data),
    g_uv_buffer_data,
    GL_STATIC_DRAW
  );
}

void UpdateTmp() {
  glUseProgram(program_id);

  glm::mat4 mvp = Locator::main_camera()->GetMvp(glm::mat4(1.0f));

  glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvp[0][0]);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);

  glUniform1i(texture_id, 0);

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

  glVertexAttribPointer(
    0,
    3,
    GL_FLOAT,
    GL_FALSE,
    0,
    (void *)0
  );

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);

  glVertexAttribPointer(
    1,
    2,
    GL_FLOAT,
    GL_FALSE,
    0,
    (void*)0
  );

  glDrawArrays(GL_TRIANGLES, 0, 12 * 3);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
}

void DestroyTmp() {
  glDeleteBuffers(1, &vertex_buffer);
  glDeleteBuffers(1, &uv_buffer);
  glDeleteTextures(1, &texture);
  glDeleteProgram(program_id);
  glDeleteVertexArrays(1, &vertex_array_id);
}
};
