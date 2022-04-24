// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_MODEL_EXPORTER_H_
#define COMET_EDITOR_ASSET_EXPORTER_MODEL_EXPORTER_H_

#include "comet_precompile.h"

#include "assimp/scene.h"

#include "comet/resource/model_resource.h"
#include "editor/asset/exporter/asset_exporter.h"

namespace comet {
namespace editor {
namespace asset {
namespace model {
class ModelExporter : public AssetExporter {
 public:
  ModelExporter() = default;
  ModelExporter(const ModelExporter&) = delete;
  ModelExporter(ModelExporter&&) = delete;
  ModelExporter& operator=(const ModelExporter&) = delete;
  ModelExporter& operator=(ModelExporter&&) = delete;
  ~ModelExporter() = default;

  bool IsCompatible(const std::string& extension) override;

 protected:
  bool AttachResourceToAssetDescr(AssetDescr& asset_descr) override;
  void LoadNode(const std::string& directory_path,
                resource::model::ModelResource& model,
                const aiNode* current_node, const aiScene* scene);
  void LoadMesh(const std::string& directory_path,
                resource::model::ModelResource& model,
                const aiMesh* current_mesh, const aiScene* scene);

  std::vector<resource::model::TextureTuple> LoadMaterialTextures(
      const std::string& resource_path, aiMaterial* material,
      aiTextureType assimp_texture_type);
};
}  // namespace model
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_MODEL_EXPORTER_H_
