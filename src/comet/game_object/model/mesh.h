// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_GAME_OBJECT_MODEL_MESH_H_
#define COMET_COMET_GAME_OBJECT_MODEL_MESH_H_

#include "boost/serialization/vector.hpp"
#include "comet/rendering/shader/shader_program.h"
#include "comet_precompile.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace comet {
namespace game_object {
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

  void Draw(std::shared_ptr<rendering::ShaderProgram>);

  template <class Archive>
  void Serialize(Archive &archive, const unsigned int file_version) {
    archive &vertices_;
    archive &indices_;
    archive &textures_;
  }

  const std::vector<Vertex> vertices() const noexcept;
  const std::vector<unsigned int> indices() const noexcept;
  const std::vector<Texture> textures() const noexcept;

 private:
  void Initialize();

  std::vector<Vertex> vertices_;
  std::vector<unsigned int> indices_;
  std::vector<Texture> textures_;
  unsigned int vao_;
  unsigned int vbo_;
  unsigned int ebo_;
};
}  // namespace game_object
}  // namespace comet

#endif  // COMET_COMET_GAME_OBJECT_MODEL_MESH_H_
