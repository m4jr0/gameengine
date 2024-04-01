// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_MODEL_MODEL_EXPORTER_H_
#define COMET_EDITOR_ASSET_EXPORTER_MODEL_MODEL_EXPORTER_H_

#include "assimp/scene.h"

#include "comet/core/essentials.h"
#include "comet/core/type/tstring.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/material_resource.h"
#include "comet/resource/model_resource.h"
#include "comet/resource/resource.h"
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

  bool IsCompatible(CTStringView extension) const override;

 protected:
  std::vector<resource::ResourceFile> GetResourceFiles(
      AssetDescr& asset_descr) const override;

 private:
  void LoadMaterials(CTStringView directory_path, const aiScene* scene,
                     std::vector<resource::ResourceFile>& resource_files) const;
  void LoadMaterialTextures(CTStringView resource_path,
                            resource::MaterialResource& material,
                            aiMaterial* raw_material,
                            aiTextureType raw_texture_type) const;
  void LoadDefaultTextures(resource::MaterialResource& material) const;
  static rendering::TextureType GetTextureType(aiTextureType raw_texture_type);
  static rendering::TextureRepeatMode GetTextureRepeatMode(
      aiTextureMapMode raw_texture_repeat_mode);

  const f32 kDefaultMaterialShininess_{8.0f};
  const aiColor3D kDefaultColor_{rendering::kColorBlack[0],
                                 rendering::kColorBlack[1],
                                 rendering::kColorBlack[2]};
};
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_MODEL_MODEL_EXPORTER_H_
