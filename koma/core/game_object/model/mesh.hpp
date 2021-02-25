// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_GAME_OBJECT_MODEL_MESH_HPP_
#define KOMA_CORE_GAME_OBJECT_MODEL_MESH_HPP_

#define LOGGER_KOMA_CORE_GAME_OBJECT_MODEL_MESH "koma_core_render"

#include <boost/serialization/vector.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <string>
#include <vector>

#include <core/render/shader/shader_program.hpp>

namespace koma {
struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 tangent;
  glm::vec3 bitangent;
  glm::vec2 texture_coordinates;
};

struct Texture {
  unsigned int id;
  std::string type;
  std::string path;
};

class Mesh {
 public:
   Mesh(std::vector<Vertex>, std::vector<unsigned int>, std::vector<Texture>);

   void Draw(std::shared_ptr<ShaderProgram>);

   const std::vector<Vertex> vertices() const noexcept;
   const std::vector<unsigned int> indices() const noexcept;
   const std::vector<Texture> textures() const noexcept;

   template<class Archive>
   void Serialize(Archive &archive, const unsigned int file_version) {
     archive & this->vertices_;
     archive & this->indices_;
     archive & this->textures_;
   }

 private:
   void Initialize();

   std::vector<Vertex> vertices_;
   std::vector<unsigned int> indices_;
   std::vector<Texture> textures_;

   unsigned int vao_;
   unsigned int vbo_;
   unsigned int ebo_;
};
}  // namespace koma

#endif  // KOMA_CORE_GAME_OBJECT_MODEL_MESH_HPP_
