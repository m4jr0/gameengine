// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_ANIMATION_ANIMATION_EXPORTER_H_
#define COMET_EDITOR_ASSET_EXPORTER_ANIMATION_ANIMATION_EXPORTER_H_

#include "assimp/Importer.hpp"
#include "assimp/scene.h"

#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/core/concurrency/job/job.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type/tstring.h"
#include "comet/resource/resource.h"
#include "editor/asset/exporter/asset_exporter.h"

namespace comet {
namespace editor {
namespace asset {
class AnimationExporter : public AssetExporter {
 public:
  AnimationExporter() = default;
  AnimationExporter(const AnimationExporter&) = delete;
  AnimationExporter(AnimationExporter&&) = delete;
  AnimationExporter& operator=(const AnimationExporter&) = delete;
  AnimationExporter& operator=(AnimationExporter&&) = delete;
  virtual ~AnimationExporter() = default;

  bool IsCompatible(CTStringView extension) const override;

 protected:
  void PopulateFiles(ResourceFilesContext& context) const override;

 private:
  struct SceneContext {
    job::IOJobDescr GenerateSceneLoadingJobDescr();
    job::JobDescr GenerateAnimationsProcessingJobDescr(job::Counter* counter);
    void AddResourceFile(const resource::ResourceFile& file);

    fiber::FiberMutex resource_mutex{};
    ResourceFiles* resource_files{nullptr};

    Assimp::Importer assimp_importer{};
    const AnimationExporter* exporter{nullptr};
    const aiScene* scene{nullptr};
    const tchar* asset_abs_path{nullptr};
    const tchar* asset_path{nullptr};
    memory::Allocator* allocator{nullptr};
  };

  static void OnSceneLoading(job::IOJobParamsHandle params_handle);
  static void OnAnimationsProcessing(job::JobParamsHandle params_handle);
};
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_ANIMATION_ANIMATION_EXPORTER_H_
