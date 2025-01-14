// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "model_exporter_utils.h"

#include "comet/geometry/geometry_common.h"

namespace comet {
namespace editor {
namespace asset {
math::Mat4 GetTransform(const math::Mat4& current_transform,
                        const aiMatrix4x4& transform_to_combine) {
  // Assimp matrices are row-major, so we compute the product from its transpose
  // directly.
  math::Mat4 result{0.0f};

  result[0][0] = current_transform[0][0] * transform_to_combine[0][0] +
                 current_transform[1][0] * transform_to_combine[0][1] +
                 current_transform[2][0] * transform_to_combine[0][2] +
                 current_transform[3][0] * transform_to_combine[0][3];

  result[0][1] = current_transform[0][1] * transform_to_combine[0][0] +
                 current_transform[1][1] * transform_to_combine[0][1] +
                 current_transform[2][1] * transform_to_combine[0][2] +
                 current_transform[3][1] * transform_to_combine[0][3];

  result[0][2] = current_transform[0][2] * transform_to_combine[0][0] +
                 current_transform[1][2] * transform_to_combine[0][1] +
                 current_transform[2][2] * transform_to_combine[0][2] +
                 current_transform[3][2] * transform_to_combine[0][3];

  result[0][3] = current_transform[0][3] * transform_to_combine[0][0] +
                 current_transform[1][3] * transform_to_combine[0][1] +
                 current_transform[2][3] * transform_to_combine[0][2] +
                 current_transform[3][3] * transform_to_combine[0][3];

  result[1][0] = current_transform[0][0] * transform_to_combine[1][0] +
                 current_transform[1][0] * transform_to_combine[1][1] +
                 current_transform[2][0] * transform_to_combine[1][2] +
                 current_transform[3][0] * transform_to_combine[1][3];

  result[1][1] = current_transform[0][1] * transform_to_combine[1][0] +
                 current_transform[1][1] * transform_to_combine[1][1] +
                 current_transform[2][1] * transform_to_combine[1][2] +
                 current_transform[3][1] * transform_to_combine[1][3];

  result[1][2] = current_transform[0][2] * transform_to_combine[1][0] +
                 current_transform[1][2] * transform_to_combine[1][1] +
                 current_transform[2][2] * transform_to_combine[1][2] +
                 current_transform[3][2] * transform_to_combine[1][3];

  result[1][3] = current_transform[0][3] * transform_to_combine[1][0] +
                 current_transform[1][3] * transform_to_combine[1][1] +
                 current_transform[2][3] * transform_to_combine[1][2] +
                 current_transform[3][3] * transform_to_combine[1][3];

  result[2][0] = current_transform[0][0] * transform_to_combine[2][0] +
                 current_transform[1][0] * transform_to_combine[2][1] +
                 current_transform[2][0] * transform_to_combine[2][2] +
                 current_transform[3][0] * transform_to_combine[2][3];

  result[2][1] = current_transform[0][1] * transform_to_combine[2][0] +
                 current_transform[1][1] * transform_to_combine[2][1] +
                 current_transform[2][1] * transform_to_combine[2][2] +
                 current_transform[3][1] * transform_to_combine[2][3];

  result[2][2] = current_transform[0][2] * transform_to_combine[2][0] +
                 current_transform[1][2] * transform_to_combine[2][1] +
                 current_transform[2][2] * transform_to_combine[2][2] +
                 current_transform[3][2] * transform_to_combine[2][3];

  result[2][3] = current_transform[0][3] * transform_to_combine[2][0] +
                 current_transform[1][3] * transform_to_combine[2][1] +
                 current_transform[2][3] * transform_to_combine[2][2] +
                 current_transform[3][3] * transform_to_combine[2][3];

  result[3][0] = current_transform[0][0] * transform_to_combine[3][0] +
                 current_transform[1][0] * transform_to_combine[3][1] +
                 current_transform[2][0] * transform_to_combine[3][2] +
                 current_transform[3][0] * transform_to_combine[3][3];

  result[3][1] = current_transform[0][1] * transform_to_combine[3][0] +
                 current_transform[1][1] * transform_to_combine[3][1] +
                 current_transform[2][1] * transform_to_combine[3][2] +
                 current_transform[3][1] * transform_to_combine[3][3];

  result[3][2] = current_transform[0][2] * transform_to_combine[3][0] +
                 current_transform[1][2] * transform_to_combine[3][1] +
                 current_transform[2][2] * transform_to_combine[3][2] +
                 current_transform[3][2] * transform_to_combine[3][3];

  result[3][3] = current_transform[0][3] * transform_to_combine[3][0] +
                 current_transform[1][3] * transform_to_combine[3][1] +
                 current_transform[2][3] * transform_to_combine[3][2] +
                 current_transform[3][3] * transform_to_combine[3][3];

  return result;
}

resource::StaticModelResource LoadStaticModel(memory::Allocator* allocator,
                                              const aiScene* scene,
                                              CTStringView path) {
  resource::StaticModelResource model{};
  model.id = resource::GenerateResourceIdFromPath(path);
  model.type_id = resource::StaticModelResource::kResourceTypeId;
  model.meshes = Array<resource::StaticMeshResource>{allocator};
  LoadStaticNode(allocator, model, scene->mRootNode, scene);
  return model;
}

void LoadStaticNode(memory::Allocator* allocator,
                    resource::StaticModelResource& model,
                    const aiNode* current_node, const aiScene* scene,
                    resource::ResourceId parent_id,
                    const math::Mat4& parent_transform) {
  auto transform{GetTransform(parent_transform, current_node->mTransformation)};
  resource::ResourceId last_mesh_id{resource::kInvalidResourceId};

  for (usize index{0}; index < current_node->mNumMeshes; ++index) {
    const auto* mesh{scene->mMeshes[current_node->mMeshes[index]]};
    last_mesh_id =
        LoadStaticMesh(allocator, model, mesh, scene, parent_id, transform);
  }

  parent_id = current_node->mNumMeshes != 1 ? parent_id : last_mesh_id;

  for (usize index{0}; index < current_node->mNumChildren; ++index) {
    LoadStaticNode(allocator, model, current_node->mChildren[index], scene,
                   parent_id, transform);
  }
}

resource::ResourceId LoadStaticMesh(memory::Allocator* allocator,
                                    resource::StaticModelResource& model,
                                    const aiMesh* current_mesh,
                                    const aiScene* scene,
                                    resource::ResourceId parent_id,
                                    const math::Mat4& transform) {
  model.meshes.PushBack(resource::StaticMeshResource{});
  auto& mesh_resource{model.meshes[model.meshes.GetSize() - 1]};
  mesh_resource.vertices = Array<geometry::Vertex>{allocator};
  mesh_resource.resource_id = model.id;
  mesh_resource.internal_id =
      static_cast<resource::ResourceId>(model.meshes.GetSize());
  mesh_resource.type = geometry::MeshType::Static;

  auto* raw_material{scene->mMaterials[current_mesh->mMaterialIndex]};
  mesh_resource.material_id =
      resource::GenerateMaterialId(raw_material->GetName().C_Str());

  auto& vertices{mesh_resource.vertices};
  vertices = Array<geometry::Vertex>{allocator};
  auto& indices{mesh_resource.indices};
  indices = Array<geometry::Index>{allocator};

  math::Vec3 min_extents{kF32Max};
  math::Vec3 max_extents{kF32Min};

  for (usize index{0}; index < current_mesh->mNumVertices; ++index) {
    vertices.PushBack(geometry::Vertex{});
    auto& vertex{vertices[vertices.GetSize() - 1]};

    if (current_mesh->mVertices) {
      vertex.position = math::Vec3{current_mesh->mVertices[index].x,
                                   current_mesh->mVertices[index].y,
                                   current_mesh->mVertices[index].z};
    }

    if (current_mesh->mNormals) {
      vertex.normal = math::Vec3{current_mesh->mNormals[index].x,
                                 current_mesh->mNormals[index].y,
                                 current_mesh->mNormals[index].z};
    }

    if (current_mesh->mTangents) {
      vertex.tangent = math::Vec3{current_mesh->mTangents[index].x,
                                  current_mesh->mTangents[index].y,
                                  current_mesh->mTangents[index].z};
    }

    if (current_mesh->mBitangents) {
      vertex.bitangent = math::Vec3{current_mesh->mBitangents[index].x,
                                    current_mesh->mBitangents[index].y,
                                    current_mesh->mBitangents[index].z};
    }

    // Does our current mesh contain texture coordinates?
    if (current_mesh->mTextureCoords[0]) {
      vertex.uv = math::Vec2{current_mesh->mTextureCoords[0][index].x,
                             current_mesh->mTextureCoords[0][index].y};
    } else {
      vertex.uv = math::Vec2{0.0f, 0.0f};
    }

    vertex.color = math::Vec4{1.0f, 1.0f, 1.0f, 1.0f};

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

  mesh_resource.transform = transform;
  mesh_resource.local_center = (max_extents + min_extents) * 0.5f;
  mesh_resource.local_max_extents = max_extents - mesh_resource.local_center;
  mesh_resource.parent_id = parent_id;

  for (usize index{0}; index < current_mesh->mNumFaces; ++index) {
    const auto& face{current_mesh->mFaces[index]};

    for (usize face_index{0}; face_index < face.mNumIndices; ++face_index) {
      indices.PushBack(face.mIndices[face_index]);
    }
  }

  return mesh_resource.internal_id;
}

resource::SkeletalModelResource LoadSkeletalModel(memory::Allocator* allocator,
                                                  const aiScene* scene,
                                                  CTStringView path) {
  resource::SkeletalModelResource model{};
  model.id = resource::GenerateResourceIdFromPath(path);
  model.type_id = resource::SkeletalModelResource::kResourceTypeId;
  model.meshes = Array<resource::SkinnedMeshResource>{allocator};
  LoadSkeletalNode(allocator, model, scene->mRootNode, scene);
  return model;
}

void LoadSkeletalNode(memory::Allocator* allocator,
                      resource::SkeletalModelResource& model,
                      const aiNode* current_node, const aiScene* scene,
                      resource::ResourceId parent_id,
                      const math::Mat4& parent_transform) {
  auto transform{GetTransform(parent_transform, current_node->mTransformation)};
  resource::ResourceId last_mesh_id{resource::kInvalidResourceId};

  for (usize index{0}; index < current_node->mNumMeshes; ++index) {
    const auto* mesh{scene->mMeshes[current_node->mMeshes[index]]};
    last_mesh_id =
        LoadSkeletalMesh(allocator, model, mesh, scene, parent_id, transform);
  }

  parent_id = current_node->mNumMeshes != 1 ? parent_id : last_mesh_id;

  for (usize index{0}; index < current_node->mNumChildren; ++index) {
    LoadSkeletalNode(allocator, model, current_node->mChildren[index], scene,
                     parent_id, transform);
  }
}

resource::ResourceId LoadSkeletalMesh(memory::Allocator* allocator,
                                      resource::SkeletalModelResource& model,
                                      const aiMesh* current_mesh,
                                      const aiScene* scene,
                                      resource::ResourceId parent_id,
                                      const math::Mat4& transform) {
  model.meshes.PushBack(resource::SkinnedMeshResource{});
  auto& mesh_resource{model.meshes[model.meshes.GetSize() - 1]};
  mesh_resource.vertices = Array<geometry::SkinnedVertex>{allocator};
  mesh_resource.resource_id = model.id;
  mesh_resource.internal_id =
      static_cast<resource::ResourceId>(model.meshes.GetSize());
  mesh_resource.type = geometry::MeshType::Skinned;

  auto* raw_material{scene->mMaterials[current_mesh->mMaterialIndex]};
  mesh_resource.material_id =
      resource::GenerateMaterialId(raw_material->GetName().C_Str());

  auto& vertices{mesh_resource.vertices};
  vertices = Array<geometry::SkinnedVertex>{allocator};
  auto& indices{mesh_resource.indices};
  indices = Array<geometry::Index>{allocator};

  math::Vec3 min_extents{kF32Max};
  math::Vec3 max_extents{kF32Min};

  for (usize index{0}; index < current_mesh->mNumVertices; ++index) {
    vertices.PushBack(geometry::SkinnedVertex{});
    auto& vertex{vertices[vertices.GetSize() - 1]};

    if (current_mesh->mVertices) {
      vertex.position = math::Vec3{current_mesh->mVertices[index].x,
                                   current_mesh->mVertices[index].y,
                                   current_mesh->mVertices[index].z};
    }

    if (current_mesh->mNormals) {
      vertex.normal = math::Vec3{current_mesh->mNormals[index].x,
                                 current_mesh->mNormals[index].y,
                                 current_mesh->mNormals[index].z};
    }

    if (current_mesh->mTangents) {
      vertex.tangent = math::Vec3{current_mesh->mTangents[index].x,
                                  current_mesh->mTangents[index].y,
                                  current_mesh->mTangents[index].z};
    }

    if (current_mesh->mBitangents) {
      vertex.bitangent = math::Vec3{current_mesh->mBitangents[index].x,
                                    current_mesh->mBitangents[index].y,
                                    current_mesh->mBitangents[index].z};
    }

    // Does our current mesh contain texture coordinates?
    if (current_mesh->mTextureCoords[0]) {
      vertex.uv = math::Vec2{current_mesh->mTextureCoords[0][index].x,
                             current_mesh->mTextureCoords[0][index].y};
    } else {
      vertex.uv = math::Vec2{0.0f, 0.0f};
    }

    vertex.color = math::Vec4{1.0f, 1.0f, 1.0f, 1.0f};

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

  mesh_resource.transform = transform;
  mesh_resource.local_center = (max_extents + min_extents) * 0.5f;
  mesh_resource.local_max_extents = max_extents - mesh_resource.local_center;
  mesh_resource.parent_id = parent_id;

  for (usize index{0}; index < current_mesh->mNumFaces; ++index) {
    const auto& face{current_mesh->mFaces[index]};

    for (usize face_index{0}; face_index < face.mNumIndices; ++face_index) {
      indices.PushBack(face.mIndices[face_index]);
    }
  }

  return mesh_resource.internal_id;
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
