// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_MODEL_MODEL_EXPORTER_H_
#define COMET_EDITOR_ASSET_EXPORTER_MODEL_MODEL_EXPORTER_H_

#include "assimp/Importer.hpp"
#include "assimp/scene.h"

#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/core/concurrency/job/job.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type/array.h"
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
  void PopulateFiles(ResourceFilesContext& context) const override;

 private:
  struct SceneContext {
    job::IOJobDescr GenerateSceneLoadingJobDescr();
    job::JobDescr GenerateModelProcessingJobDescr(job::Counter* counter);
    job::JobDescr GenerateMaterialsProcessingJobDescr(job::Counter* counter);
    void AddResourceFile(const resource::ResourceFile& file);

    fiber::FiberMutex resource_mutex{};
    ResourceFiles* resource_files{nullptr};

    Assimp::Importer assimp_importer{};
    const ModelExporter* exporter{nullptr};
    const aiScene* scene{nullptr};
    const tchar* asset_abs_path{nullptr};
    const tchar* asset_path{nullptr};
    memory::Allocator* allocator{nullptr};
  };

  static void OnSceneLoading(job::IOJobParamsHandle params_handle);
  static void OnModelProcessing(job::JobParamsHandle params_handle);
  static void OnMaterialsProcessing(job::JobParamsHandle params_handle);

  void LoadMaterials(SceneContext* data) const;
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
