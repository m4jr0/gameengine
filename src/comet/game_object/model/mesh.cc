// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "mesh.h"

#include "glad/glad.h"
#include "glm/glm.hpp"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace game_object {
Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
           std::vector<Texture> textures) {
  vertices_ = vertices;
  indices_ = indices;
  textures_ = textures;

  Initialize();
}

Mesh::Mesh(const Mesh& other)
    : vertices_(other.vertices_),
      indices_(other.indices_),
      textures_(other.textures_),
      vao_(other.vao_),
      vbo_(other.vbo_),
      ebo_(other.ebo_) {}

Mesh::Mesh(Mesh&& other) noexcept
    : vertices_(std::move(other.vertices_)),
      indices_(std::move(other.indices_)),
      textures_(std::move(other.textures_)),
      vao_(std::move(other.vao_)),
      vbo_(std::move(other.vbo_)),
      ebo_(std::move(other.ebo_)) {}

Mesh& Mesh::operator=(const Mesh& other) {
  if (this == &other) {
    return *this;
  }

  vertices_ = other.vertices_;
  indices_ = other.indices_;
  textures_ = other.textures_;
  vao_ = other.vao_;
  vbo_ = other.vbo_;
  ebo_ = other.ebo_;
  return *this;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  vertices_ = std::move(other.vertices_);
  indices_ = std::move(other.indices_);
  textures_ = std::move(other.textures_);
  vao_ = std::move(other.vao_);
  vbo_ = std::move(other.vbo_);
  ebo_ = std::move(other.ebo_);
  return *this;
}

void Mesh::Draw(std::shared_ptr<rendering::gl::ShaderProgram> shader_program) {
  std::size_t diffuse_index = 0;
  std::size_t specular_index = 0;
  std::size_t normal_index = 0;
  std::size_t height_index = 0;
  const auto textures_number = textures_.size();

  for (std::size_t index = 0; index < textures_number; ++index) {
    glActiveTexture(GL_TEXTURE0 + index);

    std::string texture_number;
    std::string texture_type = textures_[index].type;

    if (texture_type == "texture_diffuse") {
      texture_number = std::to_string(diffuse_index++);
    } else if (texture_type == "texture_specular") {
      texture_number = std::to_string(specular_index++);
    } else if (texture_type == "texture_normal") {
      texture_number = std::to_string(normal_index++);
    } else if (texture_type == "texture_height") {
      texture_number = std::to_string(height_index++);
    } else {
      COMET_LOG_GAME_OBJECT_WARNING("Unknown texture type: ", texture_type);

      continue;
    }

    shader_program->SetInt((texture_type + texture_number).c_str(), index);

    glBindTexture(GL_TEXTURE_2D, textures_[index].id);
  }

  glBindVertexArray(vao_);
  glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  glActiveTexture(GL_TEXTURE0);
}

void Mesh::Initialize() {
  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);
  glGenBuffers(1, &ebo_);

  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);

  glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(Vertex),
               &vertices_[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned int),
               &indices_[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

  glEnableVertexAttribArray(1);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void*)offsetof(Vertex, normal));

  glEnableVertexAttribArray(2);

  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void*)offsetof(Vertex, tangent));

  glEnableVertexAttribArray(3);

  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void*)offsetof(Vertex, bitangent));

  glEnableVertexAttribArray(4);

  glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void*)offsetof(Vertex, texture_coordinates));

  glBindVertexArray(0);
}

const std::vector<Vertex>& Mesh::GetVertices() const noexcept {
  return vertices_;
}

const std::vector<unsigned int>& Mesh::GetIndices() const noexcept {
  return indices_;
}

const std::vector<Texture>& Mesh::GetTextures() const noexcept {
  return textures_;
}
}  // namespace game_object
}  // namespace comet
