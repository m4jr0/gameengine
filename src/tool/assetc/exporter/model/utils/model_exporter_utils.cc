// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "assetc_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "model_exporter_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "exporter/assimp_utils.h"
#include "exporter/model/model_export_label.h"
#include "exporter/model/utils/animation_export_utils.h"

namespace comet {
namespace tool {
namespace assetc {
namespace internal {
template <typename TModelExport>
void PopulateIndexedVertices(TModelExport& model_export, const aiMesh* raw_mesh,
                             const Map<usize, ModelVertexWeights>* weights,
                             Array<geometry::SkinnedVertex>& vertices,
                             math::Vec3& min_extents, math::Vec3& max_extents) {
  COMET_ASSERT(raw_mesh != nullptr,
               "model_exporter_utils::internal::PopulateIndexedVertices",
               "raw mesh is null");

  const auto vertex_count{static_cast<usize>(raw_mesh->mNumVertices)};
  vertices = Array<geometry::SkinnedVertex>{model_export.allocator};
  vertices.Reserve(vertex_count);

  min_extents = math::Vec3{kF32Max};
  max_extents = math::Vec3{kF32Min};

  for (usize index{0}; index < vertex_count; ++index) {
    auto& vertex{vertices.EmplaceLast()};
    PopulateVertex(raw_mesh, index, vertex);

    if (weights != nullptr) {
      PopulateVertexWeights(weights->TryGet(index), vertex);
    }

    UpdateExtents(vertex, min_extents, max_extents);
  }
}

template <typename TModelExport>
resource::RawResourceId LoadMeshInternal(TModelExport& model_export,
                                         const aiMesh* raw_mesh,
                                         resource::RawResourceId parent_id,
                                         const math::Mat4& transform,
                                         geometry::MeshType mesh_type) {
  auto& model{model_export.resources->model};

  auto& mesh_resource{model.meshes.EmplaceLast()};
  mesh_resource.resource_id = model.id;
  mesh_resource.internal_id =
      static_cast<resource::RawResourceId>(model.meshes.GetSize());
  mesh_resource.type = mesh_type;
  mesh_resource.parent_id = parent_id;
  mesh_resource.transform = transform;

  auto* raw_material{model_export.scene->mMaterials[raw_mesh->mMaterialIndex]};
  mesh_resource.material_resource_id = resource::GenerateQualifiedMaterialId(
      model_export.path, raw_material->GetName().C_Str(),
      raw_mesh->mMaterialIndex);

  math::Vec3 min_extents;
  math::Vec3 max_extents;
  PopulateVertices(model_export, raw_mesh, mesh_resource.vertices, min_extents,
                   max_extents);
  PopulateIndices(model_export, raw_mesh, mesh_resource.indices);

  mesh_resource.local_center = (max_extents + min_extents) * .5f;
  mesh_resource.local_max_extents = max_extents - mesh_resource.local_center;

  return mesh_resource.internal_id;
}
}  // namespace internal

math::Mat4 GetTransform(const math::Mat4& current_transform,
                        const aiMatrix4x4& transform_to_combine) {
  const auto converted_transform{ToMat4x4(transform_to_combine)};
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
  auto& joint{joints.EmplaceLast()};
  model_export.skeleton_joint_map.Set(node_name, joint_index);
  joint.id = GenerateSkeletonJointId(node);
  joint.parent_index = parent_index;

  if (raw_bone != nullptr) {
    joint.bind_pose_inv = ToMat4x4(raw_bone->mOffsetMatrix);
  } else {
    const auto global_bind_transform{GenerateGlobalTransform(node)};
    joint.bind_pose_inv = glm::inverse(ToMat4x4(global_bind_transform));
  }

  for (u32 i{0}; i < node->mNumChildren; ++i) {
    RegisterJoint(model_export, node->mChildren[i], joint_index);
  }
}

void PopulateSkeletonJoints(SkeletalModelExport& model_export) {
  model_export.skeleton_joint_map =
      Map<const schar*, geometry::SkeletonJointIndex>::WithCapacity(
          model_export.allocator, 256);

  RegisterJoint(model_export, model_export.scene->mRootNode,
                geometry::kInvalidSkeletonJointIndex);
}

void PopulateVertices(StaticModelExport& model_export, const aiMesh* raw_mesh,
                      Array<geometry::SkinnedVertex>& vertices,
                      math::Vec3& min_extents, math::Vec3& max_extents) {
  internal::PopulateIndexedVertices(model_export, raw_mesh, nullptr, vertices,
                                    min_extents, max_extents);
}

void PopulateVertices(SkeletalModelExport& model_export, const aiMesh* raw_mesh,
                      Array<geometry::SkinnedVertex>& vertices,
                      math::Vec3& min_extents, math::Vec3& max_extents) {
  const auto weights{GenerateMeshWeights(model_export, raw_mesh)};
  internal::PopulateIndexedVertices(model_export, raw_mesh, &weights, vertices,
                                    min_extents, max_extents);
}

void PopulateIndices(ModelExport& model_export, const aiMesh* raw_mesh,
                     Array<geometry::Index>& indices) {
  COMET_ASSERT(raw_mesh != nullptr, "model_exporter_utils::PopulateIndices",
               "raw mesh is null");

  indices = Array<geometry::Index>{model_export.allocator};
  indices.Reserve(static_cast<usize>(raw_mesh->mNumFaces * 3));

  for (usize face_index{0}; face_index < raw_mesh->mNumFaces; ++face_index) {
    const auto& face{raw_mesh->mFaces[face_index]};
    COMET_ASSERT(face.mNumIndices == 3, "model_exporter_utils::PopulateIndices",
                 "mesh is not triangulated", "face_index", face_index,
                 "index_count", face.mNumIndices);

    for (usize corner_index{0}; corner_index < face.mNumIndices;
         ++corner_index) {
      indices.PushLast(face.mIndices[corner_index]);
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
  auto weight_sum{.0f};

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
  const auto vertex_count{raw_mesh->mNumVertices};

  auto weights{Map<usize, ModelVertexWeights>::WithCapacity(
      model_export.allocator, vertex_count * geometry::kMaxSkeletonJointCount)};

  for (u32 i{0}; i < raw_mesh->mNumBones; ++i) {
    const auto* raw_bone{raw_mesh->mBones[i]};
    const auto* joint_index_ptr{
        skeleton_joint_map.TryGet(raw_bone->mName.C_Str())};

    const auto joint_index{joint_index_ptr != nullptr
                               ? *joint_index_ptr
                               : geometry::kInvalidSkeletonJointIndex};

    for (u32 j{0}; j < raw_bone->mNumWeights; ++j) {
      const auto& weight{raw_bone->mWeights[j]};
      auto* weight_data{weights.TryGet(weight.mVertexId)};

      if (weight_data == nullptr) {
        weight_data = &weights.Emplace(weight.mVertexId).value;
      }

      if (weight_data->weight_count >= geometry::kMaxSkeletonJointCount) {
        COMET_LOG_WARNING(
            LoggerType::External, "model_exporter_utils::GenerateMeshWeights",
            "vertex has too many bone weights, discarding excess", "vertex_id",
            weight.mVertexId, "max_weight_count",
            geometry::kMaxSkeletonJointCount, "asset_path", model_export.path);
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
                   resource::RawResourceId parent_id,
                   const math::Mat4& parent_transform) {
  COMET_ASSERT(raw_node != nullptr, "model_exporter_utils::LoadModelNode",
               "raw node is null");

  bool is_static{false};

  switch (model_export.type) {
    case ModelExportType::Static:
      is_static = true;
      break;

    case ModelExportType::Skeletal:
      is_static = false;
      break;

    default:
      COMET_LOG_ERROR(
          LoggerType::External, "model_exporter_utils::LoadModelNode",
          "model export type is invalid", "model_export_type",
          GetModelExportTypeLabel(model_export.type), "model_export_type_value",
          ToUnderlying(model_export.type));
      return;
  }

  const auto transform{
      GetTransform(parent_transform, raw_node->mTransformation)};
  resource::RawResourceId last_mesh_id{resource::kInvalidRawResourceId};

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

resource::RawResourceId LoadMesh(StaticModelExport& model_export,
                                 const aiMesh* raw_mesh,
                                 resource::RawResourceId parent_id,
                                 const math::Mat4& transform) {
  return internal::LoadMeshInternal<StaticModelExport>(
      model_export, raw_mesh, parent_id, transform, geometry::MeshType::Static);
}

resource::RawResourceId LoadMesh(SkeletalModelExport& model_export,
                                 const aiMesh* raw_mesh,
                                 resource::RawResourceId parent_id,
                                 const math::Mat4& transform) {
  return internal::LoadMeshInternal<SkeletalModelExport>(
      model_export, raw_mesh, parent_id, transform,
      geometry::MeshType::Skinned);
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
      resource::GenerateResourceIdFromPath<resource::StaticModelResource>(path)
          .GetValue();

  resources.model.type_id = resource::StaticModelResource::kResourceTypeId;
  resources.model.meshes = Array<resource::StaticMeshResource>{allocator};

  LoadModelNode(model_export, scene->mRootNode);
  return resources;
}

SkeletalModelResources LoadSkeletalModel(memory::Allocator* allocator,
                                         const aiScene* scene,
                                         CTStringView path) {
  COMET_ASSERT(allocator != nullptr, "model_exporter_utils::LoadSkeletalModel",
               "allocator is null");
  COMET_ASSERT(scene != nullptr, "model_exporter_utils::LoadSkeletalModel",
               "scene is null");

  SkeletalModelResources resources{};

  resources.skeleton.id =
      resource::GenerateResourceIdFromPath<resource::SkeletonResource>(path)
          .GetValue();

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
          path)
          .GetValue();

  resources.model.type_id = resource::SkeletalModelResource::kResourceTypeId;
  resources.model.meshes = Array<resource::SkinnedMeshResource>{allocator};

  LoadModelNode(model_export, scene->mRootNode);

  resources.animation_clips =
      LoadAnimationClips(model_export, resources.skeleton.skeleton);

  return resources;
}

void InitializeDefaultTextureMap(resource::TextureMapResource& map,
                                 render::TextureType type) {
  map.texture_resource_id.Invalidate();
  map.type = type;
  map.u_repeat_mode = render::TextureRepeatMode::Repeat;
  map.v_repeat_mode = render::TextureRepeatMode::Repeat;
  map.w_repeat_mode = render::TextureRepeatMode::Repeat;
  map.min_filter_mode = render::TextureFilterMode::Linear;
  map.mag_filter_mode = render::TextureFilterMode::Linear;
}

render::TextureType GetTextureType(aiTextureType raw_texture_type) {
  switch (raw_texture_type) {
    case aiTextureType_BASE_COLOR:
    case aiTextureType_DIFFUSE:
      return render::TextureType::Diffuse;

    case aiTextureType_SPECULAR:
      return render::TextureType::Specular;

    case aiTextureType_NORMALS:
    case aiTextureType_HEIGHT:
      return render::TextureType::Normal;

    case aiTextureType_AMBIENT:
      return render::TextureType::Ambient;

    default:
      return render::TextureType::Unknown;
  }
}

render::TextureRepeatMode GetTextureRepeatMode(
    aiTextureMapMode raw_texture_repeat_mode) {
  switch (raw_texture_repeat_mode) {
    case aiTextureMapMode_Wrap:
      return render::TextureRepeatMode::Repeat;
    case aiTextureMapMode_Mirror:
      return render::TextureRepeatMode::MirroredRepeat;
    case aiTextureMapMode_Clamp:
      return render::TextureRepeatMode::ClampToEdge;
    case aiTextureMapMode_Decal:
      return render::TextureRepeatMode::ClampToBorder;
    default:
      return render::TextureRepeatMode::Unknown;
  }
}
}  // namespace assetc
}  // namespace tool
}  // namespace comet