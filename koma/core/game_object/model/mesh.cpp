// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "mesh.hpp"

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <utils/logger.hpp>

// Allow debugging memory leaks.
#include <debug.hpp>

namespace koma {
Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
           std::vector<Texture> textures) {
  this->vertices_ = vertices;
  this->indices_ = indices;
  this->textures_ = textures;

  this->Initialize();
}

void Mesh::Draw(std::shared_ptr<ShaderProgram> shader_program) {
  std::size_t diffuse_index = 0;
  std::size_t specular_index = 0;
  std::size_t normal_index = 0;
  std::size_t height_index = 0;
  std::size_t textures_number = this->textures_.size();

  for (std::size_t index = 0; index < textures_number; ++index) {
    glActiveTexture(GL_TEXTURE0 + index);

    std::string texture_number;
    std::string texture_type = this->textures_[index].type;

    if (texture_type == "texture_diffuse") {
      texture_number = std::to_string(diffuse_index++);
    } else if (texture_type == "texture_specular") {
      texture_number = std::to_string(specular_index++);
    } else if (texture_type == "texture_normal") {
      texture_number = std::to_string(normal_index++);
    } else if (texture_type == "texture_height") {
      texture_number = std::to_string(height_index++);
    } else {
      Logger::Get(LOGGER_KOMA_CORE_GAME_OBJECT_MODEL_MESH)->Warning(
        "Unknown texture type: ", texture_type
      );

      continue;
    }

    shader_program->SetInt(
      (texture_type + texture_number).c_str(), index
    );

    glBindTexture(GL_TEXTURE_2D, this->textures_[index].id);
  }

  glBindVertexArray(this->vao_);
  glDrawElements(GL_TRIANGLES, this->indices_.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  glActiveTexture(GL_TEXTURE0);
}

void Mesh::Initialize() {
  glGenVertexArrays(1, &this->vao_);
  glGenBuffers(1, &this->vbo_);
  glGenBuffers(1, &this->ebo_);

  glBindVertexArray(this->vao_);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo_);

  glBufferData(
    GL_ARRAY_BUFFER,
    this->vertices_.size() * sizeof(Vertex),
    &this->vertices_[0],
    GL_STATIC_DRAW
  );

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo_);

  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    this->indices_.size() * sizeof(unsigned int),
    &this->indices_[0],
    GL_STATIC_DRAW
  );

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

  glEnableVertexAttribArray(1);

  glVertexAttribPointer(
    1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
    (void *)offsetof(Vertex, normal)
  );

  glEnableVertexAttribArray(2);

  glVertexAttribPointer(
    2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
    (void *)offsetof(Vertex, tangent)
  );

  glEnableVertexAttribArray(3);

  glVertexAttribPointer(
    3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
    (void *)offsetof(Vertex, bitangent)
  );

  glEnableVertexAttribArray(4);

  glVertexAttribPointer(
    4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
    (void *)offsetof(Vertex, texture_coordinates)
  );

  glBindVertexArray(0);
}

const std::vector<Vertex> Mesh::vertices() const noexcept {
  return this->vertices_;
}

const std::vector<unsigned int> Mesh::indices() const noexcept {
  return this->indices_;
}

const std::vector<Texture> Mesh::textures() const noexcept {
  return this->textures_;
}
};  // namespace koma
