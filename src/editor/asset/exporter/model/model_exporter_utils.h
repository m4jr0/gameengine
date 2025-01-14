// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_MODEL_MODEL_EXPORTER_UTILS_H_
#define COMET_EDITOR_ASSET_EXPORTER_MODEL_MODEL_EXPORTER_UTILS_H_

#include "assimp/scene.h"

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/math/matrix.h"
#include "comet/resource/model_resource.h"
#include "comet/resource/resource.h"

namespace comet {
namespace editor {
namespace asset {
math::Mat4 GetTransform(const math::Mat4& current_transform,
                        const aiMatrix4x4& transform_to_combine);

resource::StaticModelResource LoadStaticModel(memory::Allocator* allocator,
                                              const aiScene* scene,
                                              CTStringView path);

void LoadStaticNode(
    memory::Allocator* allocator, resource::StaticModelResource& model,
    const aiNode* current_node, const aiScene* scene,
    resource::ResourceId parent_id = resource::kInvalidResourceId,
    const math::Mat4& parent_transform = math::Mat4{1.0f});

resource::ResourceId LoadStaticMesh(
    memory::Allocator* allocator, resource::StaticModelResource& model,
    const aiMesh* current_mesh, const aiScene* scene,
    resource::ResourceId parent_id = resource::kInvalidResourceId,
    const math::Mat4& transform = math::Mat4{1.0f});

resource::SkeletalModelResource LoadSkeletalModel(memory::Allocator* allocator,
                                                  const aiScene* scene,
                                                  CTStringView path);

void LoadSkeletalNode(
    memory::Allocator* allocator, resource::SkeletalModelResource& model,
    const aiNode* current_node, const aiScene* scene,
    resource::ResourceId parent_id = resource::kInvalidResourceId,
    const math::Mat4& parent_transform = math::Mat4{1.0f});

resource::ResourceId LoadSkeletalMesh(
    memory::Allocator* allocator, resource::SkeletalModelResource& model,
    const aiMesh* current_mesh, const aiScene* scene,
    resource::ResourceId parent_id = resource::kInvalidResourceId,
    const math::Mat4& transform = math::Mat4{1.0f});
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_MODEL_MODEL_EXPORTER_UTILS_H_
