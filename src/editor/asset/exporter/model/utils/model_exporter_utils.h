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
#include "comet/geometry/type/geometry_mesh_type.h"
#include "comet/geometry/type/geometry_skeleton_type.h"
#include "comet/math/matrix.h"
#include "comet/math/vector.h"
#include "comet/rendering/type/rendering_texture_type.h"
#include "comet/rendering/utils/rendering_geometry_utils.h"
#include "comet/resource/material/material_resource.h"
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

math::Mat4 GetTransform(const math::Mat4& current_transform,
                        const aiMatrix4x4& transform_to_combine);

void RegisterJoint(SkeletalModelExport& model_export, const aiNode* node,
                   geometry::SkeletonJointIndex parent_index);

void PopulateSkeletonJoints(SkeletalModelExport& model_export);

template <typename TVertex>
void PopulateVertex(const aiMesh* raw_mesh, usize index, TVertex& vertex) {
  COMET_ASSERT(raw_mesh != nullptr, "model_exporter_utils::PopulateVertex",
               "raw mesh is null");

  if (raw_mesh->mVertices != nullptr) {
    vertex.position =
        math::Vec3{raw_mesh->mVertices[index].x, raw_mesh->mVertices[index].y,
                   raw_mesh->mVertices[index].z};
  } else {
    vertex.position = math::Vec3{.0f};
  }

  math::Vec3 normal{.0f, 1.0f, .0f};

  if (raw_mesh->mNormals != nullptr) {
    normal =
        math::Vec3{raw_mesh->mNormals[index].x, raw_mesh->mNormals[index].y,
                   raw_mesh->mNormals[index].z};
  }

  vertex.normal = normal;

  if (raw_mesh->mTangents != nullptr && raw_mesh->mBitangents != nullptr) {
    const auto tangent{math::Vec3{raw_mesh->mTangents[index].x,
                                  raw_mesh->mTangents[index].y,
                                  raw_mesh->mTangents[index].z}};

    const auto bitangent{math::Vec3{raw_mesh->mBitangents[index].x,
                                    raw_mesh->mBitangents[index].y,
                                    raw_mesh->mBitangents[index].z}};

    vertex.tangent =
        rendering::GenerateTangentWithSign(normal, tangent, &bitangent);
  } else {
    vertex.tangent =
        rendering::GenerateTangentWithSign(normal, math::Vec3{.0f}, nullptr);
  }

  if (raw_mesh->mTextureCoords[0] != nullptr) {
    vertex.uv = math::Vec2{raw_mesh->mTextureCoords[0][index].x,
                           raw_mesh->mTextureCoords[0][index].y};
  } else {
    vertex.uv = math::Vec2{.0f};
  }

  vertex.color = rendering::kColorWhiteRgba;
}

template <typename TVertex>
void UpdateExtents(const TVertex& vertex, math::Vec3& min_extents,
                   math::Vec3& max_extents) {
  if (vertex.position.x < min_extents.x) {
    min_extents.x = vertex.position.x;
  }

  if (vertex.position.y < min_extents.y) {
    min_extents.y = vertex.position.y;
  }

  if (vertex.position.z < min_extents.z) {
    min_extents.z = vertex.position.z;
  }

  if (vertex.position.x > max_extents.x) {
    max_extents.x = vertex.position.x;
  }

  if (vertex.position.y > max_extents.y) {
    max_extents.y = vertex.position.y;
  }

  if (vertex.position.z > max_extents.z) {
    max_extents.z = vertex.position.z;
  }
}

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
    resource::RawResourceId parent_id = resource::kInvalidRawResourceId,
    const math::Mat4& parent_transform = math::Mat4{1.0f});

resource::RawResourceId LoadMesh(
    StaticModelExport& model_export, const aiMesh* raw_mesh,
    resource::RawResourceId parent_id = resource::kInvalidRawResourceId,
    const math::Mat4& transform = math::Mat4{1.0f});
resource::RawResourceId LoadMesh(
    SkeletalModelExport& model_export, const aiMesh* raw_mesh,
    resource::RawResourceId parent_id = resource::kInvalidRawResourceId,
    const math::Mat4& transform = math::Mat4{1.0f});

StaticModelResources LoadStaticModel(memory::Allocator* allocator,
                                     const aiScene* scene, CTStringView path);
SkeletalModelResources LoadSkeletalModel(memory::Allocator* allocator,
                                         const aiScene* scene,
                                         CTStringView path);

void InitializeDefaultTextureMap(resource::TextureMapResource& map,
                                 rendering::TextureType type);

rendering::TextureType GetTextureType(aiTextureType raw_texture_type);
rendering::TextureRepeatMode GetTextureRepeatMode(
    aiTextureMapMode raw_texture_repeat_mode);
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_MODEL_UTILS_MODEL_EXPORTER_UTILS_H_