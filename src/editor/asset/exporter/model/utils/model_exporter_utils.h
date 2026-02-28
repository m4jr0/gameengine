// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_MODEL_UTILS_MODEL_EXPORTER_UTILS_H_
#define COMET_EDITOR_ASSET_EXPORTER_MODEL_UTILS_MODEL_EXPORTER_UTILS_H_

// External. ///////////////////////////////////////////////////////////////////
#include "assimp/matrix4x4.h"
#include "assimp/mesh.h"
#include "assimp/scene.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type/array.h"
#include "comet/core/type/map.h"
#include "comet/core/type/tstring.h"
#include "comet/geometry/geometry_common.h"
#include "comet/math/matrix.h"
#include "comet/math/vector.h"
#include "comet/resource/resource.h"
#include "editor/asset/exporter/model/model_export.h"

namespace comet {
namespace editor {
namespace asset {
struct ModelVertexWeights {
  geometry::SkeletonJointIndex weight_count{0};
  geometry::SkeletonJointIndex
      joint_indices[geometry::kMaxSkeletonJointCount]{};
  f32 weights[geometry::kMaxSkeletonJointCount]{};
};

resource::ResourceId GenerateMaterialId(const ModelExport& model_export,
                                        const aiMesh* raw_mesh);
math::Mat4 GetTransform(const math::Mat4& current_transform,
                        const aiMatrix4x4& transform_to_combine);
void RegisterJoint(SkeletalModelExport& model_export, const aiNode* node,
                   geometry::SkeletonJointIndex parent_index);
void PopulateSkeletonJoints(SkeletalModelExport& model_export);
void PopulateVertex(const aiMesh* raw_mesh, usize index,
                    geometry::Vertex& vertex);
void UpdateExtents(const geometry::Vertex& vertex, math::Vec3& min_extents,
                   math::Vec3& max_extents);
void PopulateVertices(StaticModelExport& model_export, const aiMesh* raw_mesh,
                      Array<geometry::SkinnedVertex>& vertices,
                      math::Vec3& min_extents, math::Vec3& max_extents);
void PopulateVertices(SkeletalModelExport& model_export, const aiMesh* raw_mesh,
                      Array<geometry::SkinnedVertex>& vertices,
                      math::Vec3& min_extents, math::Vec3& max_extents);
void PopulateIndices(ModelExport& model_export, const aiMesh* raw_mesh,
                     Array<geometry::Index>& indices);
void PopulateVertexWeights(const ModelVertexWeights* weights,
                           geometry::SkinnedVertex& vertex);
void NormalizeVertexWeights(geometry::SkinnedVertex& vertex);
Map<usize, ModelVertexWeights> GenerateMeshWeights(
    SkeletalModelExport& model_export, const aiMesh* raw_mesh);
void LoadModelNode(
    ModelExport& model_export, const aiNode* raw_node,
    resource::ResourceId parent_id = resource::kInvalidResourceId,
    const math::Mat4& parent_transform = math::Mat4{1.0f});
resource::ResourceId LoadMesh(
    StaticModelExport& model_export, const aiMesh* raw_mesh,
    resource::ResourceId parent_id = resource::kInvalidResourceId,
    const math::Mat4& transform = math::Mat4{1.0f});
resource::ResourceId LoadMesh(
    SkeletalModelExport& model_export, const aiMesh* raw_mesh,
    resource::ResourceId parent_id = resource::kInvalidResourceId,
    const math::Mat4& transform = math::Mat4{1.0f});
StaticModelResources LoadStaticModel(memory::Allocator* allocator,
                                     const aiScene* scene, CTStringView path);
SkeletalModelResources LoadSkeletalModel(memory::Allocator* allocator,
                                         const aiScene* scene,
                                         CTStringView path);
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_MODEL_UTILS_MODEL_EXPORTER_UTILS_H_
