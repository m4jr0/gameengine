// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_MODEL_MODEL_EXPORTER_H_
#define COMET_EDITOR_ASSET_EXPORTER_MODEL_MODEL_EXPORTER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "assimp/Importer.hpp"
#include "assimp/material.h"
#include "assimp/scene.h"
#include "assimp/types.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core.h"
#include "comet/data.h"
#include "comet/render.h"
#include "exporter/asset_exporter.h"

namespace comet {
namespace tool {
namespace assetc {
class ModelExporter : public AssetExporter {
 public:
  ModelExporter() = default;
  ModelExporter(const ModelExporter&) = delete;
  ModelExporter(ModelExporter&&) = delete;
  ModelExporter& operator=(const ModelExporter&) = delete;
  ModelExporter& operator=(ModelExporter&&) = delete;
  ~ModelExporter() override = default;

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

  const f32 kDefaultMaterialShininess_{8.0f};
  const aiColor3D kDefaultColor_{render::kColorBlackRgb[0],
                                 render::kColorBlackRgb[1],
                                 render::kColorBlackRgb[2]};
};
}  // namespace assetc
}  // namespace tool
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_MODEL_MODEL_EXPORTER_H_
