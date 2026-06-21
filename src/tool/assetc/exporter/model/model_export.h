// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_MODEL_MODEL_EXPORT_H_
#define COMET_EDITOR_ASSET_EXPORTER_MODEL_MODEL_EXPORT_H_

// External. ///////////////////////////////////////////////////////////////////
#include "assimp/scene.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core.h"
#include "comet/data.h"
#include "comet/render.h"

namespace comet {
namespace tool {
namespace assetc {
struct StaticModelResources {
  resource::StaticModelResource model{};
};

struct SkeletalModelResources {
  resource::SkeletalModelResource model{};
  resource::SkeletonResource skeleton{};
  Array<resource::AnimationClipResource> animation_clips{};
};

enum class ModelExportType { Unknown = 0, Static, Skeletal };

struct ModelExport {
  ModelExportType type{ModelExportType::Unknown};
  memory::Allocator* allocator{nullptr};
  const aiScene* scene{nullptr};
  const tchar* path{nullptr};

  ~ModelExport() = default;
};

struct StaticModelExport : ModelExport {
  StaticModelResources* resources{nullptr};
};

struct SkeletalModelExport : ModelExport {
  SkeletalModelResources* resources{nullptr};
  Map<const schar*, geometry::SkeletonJointIndex> skeleton_joint_map{};
};
}  // namespace assetc
}  // namespace tool
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_MODEL_MODEL_EXPORT_H_
