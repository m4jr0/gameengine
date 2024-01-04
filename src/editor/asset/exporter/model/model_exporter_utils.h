// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_MODEL_MODEL_EXPORTER_UTILS_H_
#define COMET_EDITOR_ASSET_EXPORTER_MODEL_MODEL_EXPORTER_UTILS_H_

#include "comet_precompile.h"

#include "assimp/scene.h"

#include "comet/math/matrix.h"
#include "comet/resource/model_resource.h"
#include "comet/resource/resource.h"

namespace comet {
namespace editor {
namespace asset {
math::Mat4 GetTransform(const math::Mat4& current_transform,
                        const aiMatrix4x4& transform_to_combine);

resource::StaticModelResource LoadStaticModel(const aiScene* scene,
                                              CTStringView path);

void LoadStaticNode(
    resource::StaticModelResource& model, const aiNode* current_node,
    const aiScene* scene,
    resource::ResourceId parent_id = resource::kInvalidResourceId,
    const math::Mat4& parent_transform = math::Mat4{1.0f});

resource::ResourceId LoadStaticMesh(
    resource::StaticModelResource& model, const aiMesh* current_mesh,
    const aiScene* scene,
    resource::ResourceId parent_id = resource::kInvalidResourceId,
    const math::Mat4& transform = math::Mat4{1.0f});

resource::SkeletalModelResource LoadSkeletalModel(const aiScene* scene,
                                                  CTStringView path);

void LoadSkeletalNode(
    resource::SkeletalModelResource& model, const aiNode* current_node,
    const aiScene* scene,
    resource::ResourceId parent_id = resource::kInvalidResourceId,
    const math::Mat4& parent_transform = math::Mat4{1.0f});

resource::ResourceId LoadSkeletalMesh(
    resource::SkeletalModelResource& model, const aiMesh* current_mesh,
    const aiScene* scene,
    resource::ResourceId parent_id = resource::kInvalidResourceId,
    const math::Mat4& transform = math::Mat4{1.0f});
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_MODEL_MODEL_EXPORTER_UTILS_H_
