// Copyright 2023 m4jr0. All Rights Reserved.
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
  virtual ~ModelExporter() = default;

  bool IsCompatible(std::string_view extension) const override;

 protected:
  std::vector<resource::ResourceFile> GetResourceFiles(
      AssetDescr& asset_descr) const override;

 private:
  void LoadNode(resource::ModelResource& model, const aiNode* current_node,
                const aiScene* scene) const;
  void LoadMesh(resource::ModelResource& model, const aiMesh* current_mesh,
                const aiScene* scene) const;
  void LoadMaterials(std::string_view directory_path, const aiScene* scene,
                     std::vector<resource::ResourceFile>& resource_files) const;
  void LoadMaterialTextures(std::string_view resource_path,
                            resource::MaterialResource& material,
                            aiMaterial* raw_material,
                            aiTextureType raw_texture_type) const;
  static rendering::TextureType GetTextureType(aiTextureType raw_texture_type);
  static rendering::TextureRepeatMode GetTextureRepeatMode(
      aiTextureMapMode raw_texture_repeat_mode);

  const f32 kDefaultMaterialShininess_{8.0f};
  const aiColor3D kDefaultColor_{0.f, 0.f, 0.f};
};
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_MODEL_EXPORTER_H_
