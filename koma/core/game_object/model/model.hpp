// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_GAME_OBJECT_MODEL_MODEL_HPP_
#define KOMA_CORE_GAME_OBJECT_MODEL_MODEL_HPP_

#define LOGGER_KOMA_CORE_GAME_OBJECT_MODEL_MODEL "koma_core_render"

#include "mesh.hpp"

#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <string>
#include <vector>

#include <core/game_object/component.hpp>
#include <core/game_object/physics/transform.hpp>
#include <core/render/shader/shader_program.hpp>

namespace koma {
class Model : public Component {
 public:
  Model(const std::string &, std::shared_ptr<ShaderProgram> = nullptr);

  void Draw(std::shared_ptr<ShaderProgram>);
  void Initialize() override;
  void Destroy() override;
  void Update() override;

 private:
  std::vector<Mesh> meshes_;
  std::string path_;
  std::string directory_;
  std::shared_ptr<Transform> transform_ = nullptr;
  std::shared_ptr<ShaderProgram> shader_program_ = nullptr;

  void LoadModel();
  void LoadNode(aiNode *, const aiScene *);
  Mesh LoadMesh(aiMesh *, const aiScene *);

  std::vector<Texture> LoadMaterialTextures(aiMaterial *, aiTextureType,
                                            std::string);

  std::vector<Texture> loaded_textures_;
};
};  // namespace koma

#endif  // KOMA_CORE_GAME_OBJECT_MODEL_MODEL_HPP_
