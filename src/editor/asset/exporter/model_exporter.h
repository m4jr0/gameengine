// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_MODEL_EXPORTER_H_
#define COMET_EDITOR_ASSET_EXPORTER_MODEL_EXPORTER_H_

#include "comet_precompile.h"

#include "assimp/scene.h"

#include "comet/resource/material_resource.h"
#include "comet/resource/model_resource.h"
#include "editor/asset/exporter/asset_exporter.h"

namespace comet {
namespace editor {
namespace asset {
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
  std::vector<resource::ResourceFile> GetResourceFiles(
      AssetDescr& asset_descr) override;
  void LoadNode(resource::ModelResource& model, const aiNode* current_node,
                const aiScene* scene);
  void LoadMesh(resource::ModelResource& model, const aiMesh* current_mesh,
                const aiScene* scene);
  void LoadMaterials(const std::string& directory_path, const aiScene* scene,
                     std::vector<resource::ResourceFile>& resource_files);
  void LoadMaterialTextures(const std::string& resource_path,
                            resource::MaterialResource& material,
                            aiMaterial* raw_material,
                            aiTextureType raw_texture_type);
  rendering::TextureType GetTextureType(aiTextureType raw_texture_type);
};
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_MODEL_EXPORTER_H_
