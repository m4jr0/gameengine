// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_MODEL_MODEL_EXPORT_H_
#define COMET_EDITOR_ASSET_EXPORTER_MODEL_MODEL_EXPORT_H_

#include "assimp/scene.h"

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type/array.h"
#include "comet/core/type/map.h"
#include "comet/geometry/geometry_common.h"
#include "comet/resource/animation_resource.h"
#include "comet/resource/model_resource.h"

namespace comet {
namespace editor {
namespace asset {
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

  virtual ~ModelExport() = default;
};

struct StaticModelExport : ModelExport {
  StaticModelResources* resources{nullptr};
};

struct SkeletalModelExport : ModelExport {
  SkeletalModelResources* resources{nullptr};
  Map<const schar*, geometry::SkeletonJointIndex> skeleton_joint_map{};
};
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_MODEL_MODEL_EXPORT_H_
