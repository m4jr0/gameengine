// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_GAME_OBJECT_MODEL_MODEL_H_
#define COMET_COMET_GAME_OBJECT_MODEL_MODEL_H_

#include "assimp/scene.h"
#include "comet/game_object/component.h"
#include "comet/game_object/model/mesh.h"
#include "comet/game_object/physics/transform.h"
#include "comet/rendering/shader/shader_program.h"
#include "comet_precompile.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace comet {
namespace game_object {
class Model : public Component {
 public:
  Model(const std::string&,
        std::shared_ptr<rendering::ShaderProgram> = nullptr);
  Model(const Model&);
  Model(Model&&) noexcept;
  Model& operator=(const Model&);
  Model& operator=(Model&&) noexcept;
  virtual ~Model() = default;

  virtual std::shared_ptr<Component> Clone() const override;
  void Initialize() override;
  void Destroy() override;
  void Update() override;
  void Draw(std::shared_ptr<rendering::ShaderProgram>);

  template <class Archive>
  void Serialize(Archive& archive, const unsigned int file_version) {
    archive& meshes_;
    archive& path_;
    archive& directory_;
    archive& transform_;
    archive& shader_program_;
    archive& loaded_textures_;
  }

 private:
  void LoadModel();
  void LoadNode(const aiNode*, const aiScene*);
  Mesh LoadMesh(const aiMesh*, const aiScene*);

  std::vector<Texture> LoadMaterialTextures(aiMaterial*, aiTextureType,
                                            const std::string&);

  std::vector<Mesh> meshes_;
  std::string path_;
  std::string directory_;
  std::shared_ptr<Transform> transform_ = nullptr;
  std::shared_ptr<rendering::ShaderProgram> shader_program_ = nullptr;
  std::vector<Texture> loaded_textures_;
};
}  // namespace game_object
}  // namespace comet

#endif  // COMET_COMET_GAME_OBJECT_MODEL_MODEL_H_
