// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "model_exporter_utils.h"

#include <type_traits>

#include "comet/rendering/rendering_common.h"
#include "comet/resource/material_resource.h"
#include "comet/resource/model_resource.h"
#include "editor/asset/exporter/assimp_utils.h"
#include "editor/asset/exporter/model/utils/animation_export_utils.h"

namespace comet {
namespace editor {
namespace asset {
resource::ResourceId GenerateMaterialId(const ModelExport& model_export,
                                        const aiMesh* raw_mesh) {
  auto* raw_material{model_export.scene->mMaterials[raw_mesh->mMaterialIndex]};
  return resource::GenerateMaterialId(raw_material->GetName().C_Str());
}

math::Mat4 GetTransform(const math::Mat4& current_transform,
                        const aiMatrix4x4& transform_to_combine) {
  auto converted_transform{ToMat4x4(transform_to_combine)};
  return current_transform * converted_transform;
}

void RegisterJoint(SkeletalModelExport& model_export, const aiNode* node,
                   geometry::SkeletonJointIndex parent_index) {
  const auto* scene{model_export.scene};
  const auto* node_name{node->mName.C_Str()};
  const aiBone* raw_bone{nullptr};

  for (u32 i{0}; i < scene->mNumMeshes && raw_bone == nullptr; ++i) {
    const auto* mesh{scene->mMeshes[i]};

    for (u32 j{0}; j < mesh->mNumBones; ++j) {
      const auto* current_raw_bone{mesh->mBones[j]};

      if (AreStringsEqual(current_raw_bone->mName.C_Str(), node_name)) {
        raw_bone = current_raw_bone;
        break;
      }
    }
  }

  auto& joints{model_export.resources->skeleton.skeleton.joints};
  geometry::SkeletonJointIndex joint_index{
      static_cast<geometry::SkeletonJointIndex>(joints.GetSize())};
  auto& joint{joints.EmplaceBack()};
  model_export.skeleton_joint_map[node_name] = joint_index;
  joint.id = GenerateSkeletonJointId(node);
  joint.parent_index = parent_index;

  if (raw_bone != nullptr) {
    joint.bind_pose_inv = ToMat4x4(raw_bone->mOffsetMatrix);
  } else {
    auto global_bind_transform{GenerateGlobalTransform(node)};
    joint.bind_pose_inv = glm::inverse(ToMat4x4(global_bind_transform));
  }

  for (u32 i{0}; i < node->mNumChildren; ++i) {
    RegisterJoint(model_export, node->mChildren[i], joint_index);
  }
}

void PopulateSkeletonJoints(SkeletalModelExport& model_export) {
  model_export.skeleton_joint_map =
      Map<const schar*, geometry::SkeletonJointIndex>{model_export.allocator,
                                                      256};

  RegisterJoint(model_export, model_export.scene->mRootNode,
                geometry::kInvalidSkeletonJointIndex);
}

void PopulateVertex(const aiMesh* raw_mesh, usize index,
                    geometry::Vertex& vertex) {
  if (raw_mesh->mVertices) {
    vertex.position =
        math::Vec3{raw_mesh->mVertices[index].x, raw_mesh->mVertices[index].y,
                   raw_mesh->mVertices[index].z};
  }

  if (raw_mesh->mNormals) {
    vertex.normal =
        math::Vec3{raw_mesh->mNormals[index].x, raw_mesh->mNormals[index].y,
                   raw_mesh->mNormals[index].z};
  }

  if (raw_mesh->mTangents) {
    vertex.tangent =
        math::Vec3{raw_mesh->mTangents[index].x, raw_mesh->mTangents[index].y,
                   raw_mesh->mTangents[index].z};
  }

  if (raw_mesh->mBitangents) {
    vertex.bitangent = math::Vec3{raw_mesh->mBitangents[index].x,
                                  raw_mesh->mBitangents[index].y,
                                  raw_mesh->mBitangents[index].z};
  }

  // Does our current mesh contain texture coordinates?
  if (raw_mesh->mTextureCoords[0]) {
    vertex.uv = math::Vec2{raw_mesh->mTextureCoords[0][index].x,
                           raw_mesh->mTextureCoords[0][index].y};
  } else {
    vertex.uv = math::Vec2{0.0f, 0.0f};
  }

  vertex.color = rendering::kColorWhiteRgba;
}

void UpdateExtents(const geometry::Vertex& vertex, math::Vec3& min_extents,
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
                      math::Vec3& min_extents, math::Vec3& max_extents) {
  auto vertex_count{raw_mesh->mNumVertices};
  vertices = Array<geometry::SkinnedVertex>{model_export.allocator};
  vertices.Reserve(vertex_count);

  min_extents = math::Vec3{kF32Max};
  max_extents = math::Vec3{kF32Min};

  // Currently, the AABBs used for animated models are derived directly from
  // their static bind-pose geometry. While this approach is simple, it does not
  // always account for the full extent of vertex deformation during animation
  // (e.g., limbs stretching out or rotating far from the rest pose).

  // For more accurate frustum culling and to avoid visible popping artifacts,
  // AABBs should ideally be authored by artists (or generated programatically)
  // using the model’s full animation set: typically by evaluating the pose that
  // results in the maximum spatial extent. This ensures conservative but
  // correct bounding volumes that fully enclose the skinned mesh at all times.
  for (usize index{0}; index < vertex_count; ++index) {
    auto& vertex{vertices.EmplaceBack()};
    PopulateVertex(raw_mesh, index, vertex);
    UpdateExtents(vertex, min_extents, max_extents);
  }
}

void PopulateVertices(SkeletalModelExport& model_export, const aiMesh* raw_mesh,
                      Array<geometry::SkinnedVertex>& vertices,
                      math::Vec3& min_extents, math::Vec3& max_extents) {
  auto vertex_count{raw_mesh->mNumVertices};
  vertices = Array<geometry::SkinnedVertex>{model_export.allocator};
  vertices.Reserve(vertex_count);

  auto weights{GenerateMeshWeights(model_export, raw_mesh)};

  min_extents = math::Vec3{kF32Max};
  max_extents = math::Vec3{kF32Min};

  for (usize index{0}; index < vertex_count; ++index) {
    auto& vertex{vertices.EmplaceBack()};
    PopulateVertex(raw_mesh, index, vertex);
    UpdateExtents(vertex, min_extents, max_extents);
    PopulateVertexWeights(weights.TryGet(index), vertex);
  }
}

void PopulateIndices(ModelExport& model_export, const aiMesh* raw_mesh,
                     Array<geometry::Index>& indices) {
  indices = Array<geometry::Index>{model_export.allocator};
  indices.Reserve(static_cast<usize>(raw_mesh->mNumFaces * 3));

  for (usize index{0}; index < raw_mesh->mNumFaces; ++index) {
    const auto& face{raw_mesh->mFaces[index]};

    for (usize face_index{0}; face_index < face.mNumIndices; ++face_index) {
      indices.PushBack(face.mIndices[face_index]);
    }
  }
}

void PopulateVertexWeights(const ModelVertexWeights* weights,
                           geometry::SkinnedVertex& vertex) {
  if (weights == nullptr) {
    return;
  }

  for (geometry::SkeletonJointIndex i{0}; i < weights->weight_count; ++i) {
    vertex.joint_indices[i] = weights->joint_indices[i];
    vertex.joint_weights[i] = weights->weights[i];
  }

  NormalizeVertexWeights(vertex);
}

void NormalizeVertexWeights(geometry::SkinnedVertex& vertex) {
  f32 weight_sum{.0f};

  for (u32 i{0}; i < geometry::kMaxSkeletonJointCount; ++i) {
    weight_sum += vertex.joint_weights[i];
  }

  if (weight_sum == .0f) {
    return;
  }

  for (u32 i{0}; i < geometry::kMaxSkeletonJointCount; ++i) {
    vertex.joint_weights[i] /= weight_sum;
  }
}

Map<usize, ModelVertexWeights> GenerateMeshWeights(
    SkeletalModelExport& model_export, const aiMesh* raw_mesh) {
  auto& skeleton_joint_map{model_export.skeleton_joint_map};
  auto vertex_count{raw_mesh->mNumVertices};

  Map<usize, ModelVertexWeights> weights{
      model_export.allocator, vertex_count * geometry::kMaxSkeletonJointCount};

  for (u32 i{0}; i < raw_mesh->mNumBones; ++i) {
    const auto* raw_bone{raw_mesh->mBones[i]};
    const auto* joint_index_ptr{
        skeleton_joint_map.TryGet(raw_bone->mName.C_Str())};
    auto joint_index{joint_index_ptr != nullptr
                         ? *joint_index_ptr
                         : geometry::kInvalidSkeletonJointIndex};

    for (u32 j{0}; j < raw_bone->mNumWeights; ++j) {
      const auto& weight{raw_bone->mWeights[j]};
      auto* weight_data{weights.TryGet(weight.mVertexId)};

      if (weight_data == nullptr) {
        weight_data = &weights.Emplace(weight.mVertexId).value;
      }

      if (weight_data->weight_count >= geometry::kMaxSkeletonJointCount) {
        COMET_LOG_GLOBAL_WARNING(
            "Too many bone weights for vertex ", weight.mVertexId, " (max is ",
            geometry::kMaxSkeletonJointCount, ") in asset at path ",
            model_export.path, ". Discarding exceeding weights.");
        continue;
      }

      weight_data->joint_indices[weight_data->weight_count] = joint_index;
      weight_data->weights[weight_data->weight_count] = weight.mWeight;
      ++weight_data->weight_count;
    }
  }

  return weights;
}

void LoadModelNode(ModelExport& model_export, const aiNode* raw_node,
                   resource::ResourceId parent_id,
                   const math::Mat4& parent_transform) {
  bool is_static;

  switch (model_export.type) {
    case ModelExportType::Static:
      is_static = true;
      break;
    case ModelExportType::Skeletal:
      is_static = false;
      break;
    default:
      COMET_LOG_GLOBAL_ERROR(
          "Unknown or unsupported model export type: ",
          static_cast<std::underlying_type_t<ModelExportType>>(
              model_export.type),
          "!");
      return;
  }

  auto transform{GetTransform(parent_transform, raw_node->mTransformation)};
  resource::ResourceId last_mesh_id{resource::kInvalidResourceId};

  for (usize index{0}; index < raw_node->mNumMeshes; ++index) {
    const auto* raw_mesh{model_export.scene->mMeshes[raw_node->mMeshes[index]]};

    if (is_static) {
      last_mesh_id = LoadMesh(static_cast<StaticModelExport&>(model_export),
                              raw_mesh, parent_id, transform);
    } else {
      last_mesh_id = LoadMesh(static_cast<SkeletalModelExport&>(model_export),
                              raw_mesh, parent_id, transform);
    }
  }

  parent_id = raw_node->mNumMeshes != 1 ? parent_id : last_mesh_id;

  for (usize index{0}; index < raw_node->mNumChildren; ++index) {
    LoadModelNode(model_export, raw_node->mChildren[index], parent_id,
                  transform);
  }
}

resource::ResourceId LoadMesh(StaticModelExport& model_export,
                              const aiMesh* raw_mesh,
                              resource::ResourceId parent_id,
                              const math::Mat4& transform) {
  auto& model{model_export.resources->model};

  auto& mesh_resource{model.meshes.EmplaceBack()};
  mesh_resource.resource_id = model.id;
  mesh_resource.internal_id =
      static_cast<resource::ResourceId>(model.meshes.GetSize());
  mesh_resource.type = geometry::MeshType::Skinned;
  mesh_resource.parent_id = parent_id;
  mesh_resource.transform = transform;
  mesh_resource.material_id = GenerateMaterialId(model_export, raw_mesh);

  math::Vec3 min_extents;
  math::Vec3 max_extents;
  PopulateVertices(model_export, raw_mesh, mesh_resource.vertices, min_extents,
                   max_extents);
  PopulateIndices(model_export, raw_mesh, mesh_resource.indices);

  mesh_resource.local_center = (max_extents + min_extents) * 0.5f;
  mesh_resource.local_max_extents = max_extents - mesh_resource.local_center;

  return mesh_resource.internal_id;
}

resource::ResourceId LoadMesh(SkeletalModelExport& model_export,
                              const aiMesh* raw_mesh,
                              resource::ResourceId parent_id,
                              const math::Mat4& transform) {
  auto& model{model_export.resources->model};

  auto& mesh_resource{model.meshes.EmplaceBack()};
  mesh_resource.resource_id = model.id;
  mesh_resource.internal_id =
      static_cast<resource::ResourceId>(model.meshes.GetSize());
  mesh_resource.type = geometry::MeshType::Skinned;
  mesh_resource.parent_id = parent_id;
  mesh_resource.transform = transform;
  mesh_resource.material_id = GenerateMaterialId(model_export, raw_mesh);

  math::Vec3 min_extents;
  math::Vec3 max_extents;
  PopulateVertices(model_export, raw_mesh, mesh_resource.vertices, min_extents,
                   max_extents);
  PopulateIndices(model_export, raw_mesh, mesh_resource.indices);

  mesh_resource.local_center = (max_extents + min_extents) * 0.5f;
  mesh_resource.local_max_extents = max_extents - mesh_resource.local_center;

  return mesh_resource.internal_id;
}

StaticModelResources LoadStaticModel(memory::Allocator* allocator,
                                     const aiScene* scene, CTStringView path) {
  StaticModelResources resources{};

  StaticModelExport model_export{};
  model_export.type = ModelExportType::Static;
  model_export.allocator = allocator;
  model_export.scene = scene;
  model_export.path = path.GetCTStr();
  model_export.resources = &resources;

  resources.model.id =
      resource::GenerateResourceIdFromPath<resource::StaticModelResource>(path);
  resources.model.type_id = resource::StaticModelResource::kResourceTypeId;
  resources.model.meshes = Array<resource::StaticMeshResource>{allocator};
  LoadModelNode(model_export, scene->mRootNode);
  return resources;
}

SkeletalModelResources LoadSkeletalModel(memory::Allocator* allocator,
                                         const aiScene* scene,
                                         CTStringView path) {
  SkeletalModelResources resources{};
  resources.skeleton.id =
      resource::GenerateResourceIdFromPath<resource::SkeletonResource>(path);
  resources.skeleton.type_id = resource::SkeletonResource::kResourceTypeId;
  resources.skeleton.skeleton.id = resources.skeleton.id;
  resources.skeleton.skeleton.joints =
      Array<geometry::SkeletonJoint>{allocator};

  SkeletalModelExport model_export{};
  model_export.type = ModelExportType::Skeletal;
  model_export.allocator = allocator;
  model_export.scene = scene;
  model_export.path = path.GetCTStr();
  model_export.resources = &resources;

  PopulateSkeletonJoints(model_export);

  resources.model.id =
      resource::GenerateResourceIdFromPath<resource::SkeletalModelResource>(
          path);
  resources.model.type_id = resource::SkeletalModelResource::kResourceTypeId;
  resources.model.meshes = Array<resource::SkinnedMeshResource>{allocator};
  LoadModelNode(model_export, scene->mRootNode);

  resources.animation_clips =
      LoadAnimationClips(model_export, resources.skeleton.skeleton);

  return resources;
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
