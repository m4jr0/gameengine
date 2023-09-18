// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "model_exporter.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"

#include "comet/core/file_system.h"
#include "comet/core/generator.h"
#include "comet/core/memory/memory.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/resource_manager.h"
#include "comet/resource/texture_resource.h"

namespace comet {
namespace editor {
namespace asset {
bool ModelExporter::IsCompatible(CTStringView extension) const {
  return extension == COMET_TCHAR("obj") || extension == COMET_TCHAR("fbx");
}

std::vector<resource::ResourceFile> ModelExporter::GetResourceFiles(
    AssetDescr& asset_descr) const {
  std::vector<resource::ResourceFile> resource_files{};
  Assimp::Importer importer;

#ifdef COMET_WIDE_TCHAR
  auto* asset_abs_path{
      GenerateForOneFrame<schar>(asset_descr.asset_abs_path.GetCTStr(),
                                 asset_descr.asset_abs_path.GetLength())};
#else
  auto* asset_abs_path{asset_descr.asset_abs_path.GetCTStr()};
#endif  // COMET_WIDE_TCHAR

  const auto* scene{importer.ReadFile(
      asset_abs_path,
      aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace)};

  if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      scene->mRootNode == nullptr) {
    COMET_LOG_RESOURCE_ERROR("Assimp error: ", importer.GetErrorString());
    return resource_files;
  }

  // N material resources, and 1 model.
  resource_files.reserve(scene->mNumMaterials + 1);

  resource::ModelResource model;
  model.id = resource::GenerateResourceIdFromPath(asset_descr.asset_path);
  model.type_id = resource::ModelResource::kResourceTypeId;
  LoadNode(model, scene->mRootNode, scene);

  resource_files.push_back(resource::ResourceManager::Get().GetResourceFile(
      model, compression_mode_));

  LoadMaterials(GetDirectoryPath(asset_descr.asset_abs_path), scene,
                resource_files);

  return resource_files;
}

math::Mat4 ModelExporter::GetTransform(
    const math::Mat4& current_transform,
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

void ModelExporter::LoadNode(resource::ModelResource& model,
                             const aiNode* current_node, const aiScene* scene,
                             resource::ResourceId parent_id,
                             const math::Mat4& parent_transform) const {
  auto transform{GetTransform(parent_transform, current_node->mTransformation)};
  resource::ResourceId last_mesh_id{resource::kInvalidResourceId};

  for (uindex index{0}; index < current_node->mNumMeshes; ++index) {
    const auto* mesh{scene->mMeshes[current_node->mMeshes[index]]};
    last_mesh_id = LoadMesh(model, mesh, scene, parent_id, transform);
  }

  parent_id = current_node->mNumMeshes != 1 ? parent_id : last_mesh_id;

  for (uindex index{0}; index < current_node->mNumChildren; ++index) {
    LoadNode(model, current_node->mChildren[index], scene, parent_id,
             transform);
  }
}

resource::ResourceId ModelExporter::LoadMesh(
    resource::ModelResource& model, const aiMesh* current_mesh,
    const aiScene* scene, resource::ResourceId parent_id,
    const math::Mat4& transform) const {
  model.meshes.push_back({});
  auto& mesh_resource{model.meshes[model.meshes.size() - 1]};
  mesh_resource.resource_id = model.id;
  mesh_resource.internal_id = model.meshes.size();

  auto* raw_material{scene->mMaterials[current_mesh->mMaterialIndex]};
  mesh_resource.material_id =
      resource::GenerateMaterialId(raw_material->GetName().C_Str());

  auto& vertices{mesh_resource.vertices};
  auto& indices{mesh_resource.indices};

  math::Vec3 min_extents{std::numeric_limits<float>::max()};
  math::Vec3 max_extents{std::numeric_limits<float>::lowest()};

  for (uindex index{0}; index < current_mesh->mNumVertices; ++index) {
    vertices.push_back({});
    auto& vertex{vertices[vertices.size() - 1]};

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

  for (uindex index{0}; index < current_mesh->mNumFaces; ++index) {
    const auto& face{current_mesh->mFaces[index]};

    for (uindex face_index{0}; face_index < face.mNumIndices; ++face_index) {
      indices.push_back(face.mIndices[face_index]);
    }
  }

  return mesh_resource.internal_id;
}

void ModelExporter::LoadMaterialTextures(CTStringView resource_path,
                                         resource::MaterialResource& material,
                                         aiMaterial* raw_material,
                                         aiTextureType raw_texture_type) const {
  const auto texture_count{raw_material->GetTextureCount(raw_texture_type)};

  if (texture_count == 0) {
    return;
  }

  auto texture_type{GetTextureType(raw_texture_type)};

  if (texture_count > 1) {
    COMET_LOG_GLOBAL_WARNING("Texture count for type \"",
                             rendering::GetTextureTypeLabel(texture_type),
                             "\" is greater than 1. This is not supported. "
                             "Ignoring excess textures.");
  }

  resource::TextureMap* map{nullptr};

  switch (texture_type) {
    case rendering::TextureType::Diffuse:
      map = &material.descr.diffuse_map;
      break;
    case rendering::TextureType::Specular:
      map = &material.descr.specular_map;
      break;
      // TODO(m4jr0): Check behavior. Apparently, Assimp might process normal
      // textures as height ones. See
      // https://github.com/assimp/assimp/issues/430
    case rendering::TextureType::Normal:
      map = &material.descr.normal_map;
      break;
    default:
      COMET_LOG_GLOBAL_WARNING("Unsupported texture type: ",
                               rendering::GetTextureTypeLabel(texture_type),
                               ". Ignoring.");
      break;
  }

  if (map == nullptr) {
    return;
  }

  aiString texture_path;
  raw_material->GetTexture(raw_texture_type, 0, &texture_path);
  map->texture_id = resource::GenerateResourceIdFromPath(
      resource_path / GetTmpTChar(texture_path.C_Str()));
  map->type = texture_type;

  aiTextureMapMode raw_texture_repeat_mode;

  if (raw_material->Get(AI_MATKEY_MAPPINGMODE_U(raw_texture_type, 0),
                        raw_texture_repeat_mode) != AI_SUCCESS) {
    COMET_LOG_GLOBAL_DEBUG(
        "Could not get texture repeat mode for U. Setting it to repeat mode.");
    raw_texture_repeat_mode = aiTextureMapMode_Wrap;
  }

  map->u_repeat_mode = GetTextureRepeatMode(raw_texture_repeat_mode);

  if (raw_material->Get(AI_MATKEY_MAPPINGMODE_V(raw_texture_type, 0),
                        raw_texture_repeat_mode) != AI_SUCCESS) {
    COMET_LOG_GLOBAL_DEBUG(
        "Could not get texture repeat mode for V. Setting it to repeat "
        "mode.");
    raw_texture_repeat_mode = aiTextureMapMode_Wrap;
  }

  map->v_repeat_mode = GetTextureRepeatMode(raw_texture_repeat_mode);

  // TODO(m4jr0): Handle filter modes better.
  map->min_filter_mode = rendering::TextureFilterMode::Linear;
  map->mag_filter_mode = rendering::TextureFilterMode::Linear;
}

void ModelExporter::LoadDefaultTextures(
    resource::MaterialResource& material) const {
  if (material.descr.diffuse_map.type == rendering::TextureType::Unknown) {
    auto& map{material.descr.diffuse_map};
    map.texture_id = resource::kInvalidResourceId;
    map.type = rendering::TextureType::Diffuse;
    map.u_repeat_mode = rendering::TextureRepeatMode::Repeat;
    map.v_repeat_mode = rendering::TextureRepeatMode::Repeat;
    map.min_filter_mode = rendering::TextureFilterMode::Linear;
    map.mag_filter_mode = rendering::TextureFilterMode::Linear;
  }

  if (material.descr.specular_map.type == rendering::TextureType::Unknown) {
    auto& map{material.descr.specular_map};
    map.texture_id = resource::kInvalidResourceId;
    map.type = rendering::TextureType::Specular;
    map.u_repeat_mode = rendering::TextureRepeatMode::Repeat;
    map.v_repeat_mode = rendering::TextureRepeatMode::Repeat;
    map.min_filter_mode = rendering::TextureFilterMode::Linear;
    map.mag_filter_mode = rendering::TextureFilterMode::Linear;
  }

  if (material.descr.normal_map.type == rendering::TextureType::Unknown) {
    auto& map{material.descr.normal_map};
    map.texture_id = resource::kInvalidResourceId;
    map.type = rendering::TextureType::Normal;
    map.u_repeat_mode = rendering::TextureRepeatMode::Repeat;
    map.v_repeat_mode = rendering::TextureRepeatMode::Repeat;
    map.min_filter_mode = rendering::TextureFilterMode::Linear;
    map.mag_filter_mode = rendering::TextureFilterMode::Linear;
  }
}

void ModelExporter::LoadMaterials(
    CTStringView directory_path, const aiScene* scene,
    std::vector<resource::ResourceFile>& resource_files) const {
  const auto resource_path{GetRelativePath(directory_path, root_asset_path_)};
  constexpr std::array<aiTextureType, 4> exported_texture_types{
      {aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_HEIGHT,
       aiTextureType_AMBIENT}};

  for (uindex i{0}; i < scene->mNumMaterials; ++i) {
    const auto raw_material{scene->mMaterials[i]};

    COMET_LOG_GLOBAL_DEBUG("Material \"", raw_material->GetName().C_Str(),
                           "\" found.");

    resource::MaterialResource material{};
    CopyMemory(material.descr.shader_name, "default_shader", 14);

    if (aiGetMaterialFloat(raw_material, AI_MATKEY_SHININESS,
                           &material.descr.shininess) != AI_SUCCESS) {
      COMET_LOG_GLOBAL_WARNING(
          "Could not retrieve shininess property from material. Setting it to ",
          kDefaultMaterialShininess_);
      material.descr.shininess = kDefaultMaterialShininess_;
    }

    aiColor3D color;

    if (raw_material->Get(AI_MATKEY_COLOR_DIFFUSE, color) != AI_SUCCESS) {
      COMET_LOG_GLOBAL_WARNING(
          "Could not retrieve diffuse color property from material. Setting it "
          "to (",
          kDefaultColor_.r, ", ", kDefaultColor_.g, ", ", kDefaultColor_.b,
          ").");

      color = kDefaultColor_;
    }

    material.descr.diffuse_color = math::Vec4(color.r, color.b, color.g, 1.0f);

    for (auto raw_texture_type : exported_texture_types) {
      LoadMaterialTextures(resource_path, material, raw_material,
                           raw_texture_type);
    }

    LoadDefaultTextures(material);
    material.id = resource::GenerateMaterialId(raw_material->GetName().C_Str());
    material.type_id = resource::MaterialResource::kResourceTypeId;

    resource_files.push_back(resource::ResourceManager::Get().GetResourceFile(
        material, compression_mode_));
  }
}

rendering::TextureType ModelExporter::GetTextureType(
    aiTextureType raw_texture_type) {
  switch (raw_texture_type) {
    case aiTextureType_DIFFUSE: {
      return rendering::TextureType::Diffuse;
    }
    case aiTextureType_SPECULAR: {
      return rendering::TextureType::Specular;
    }
    case aiTextureType_HEIGHT: {
      return rendering::TextureType::Normal;
    }
    case aiTextureType_AMBIENT: {
      return rendering::TextureType::Ambient;
    }
    case aiTextureType_BASE_COLOR: {
      return rendering::TextureType::Color;
    }
  }

  return rendering::TextureType::Unknown;
}

rendering::TextureRepeatMode ModelExporter::GetTextureRepeatMode(
    aiTextureMapMode raw_texture_repeat_mode) {
  switch (raw_texture_repeat_mode) {
    case aiTextureMapMode_Wrap:
      return rendering::TextureRepeatMode::Repeat;
    case aiTextureMapMode_Mirror:
      return rendering::TextureRepeatMode::MirroredRepeat;
    case aiTextureMapMode_Clamp:
      return rendering::TextureRepeatMode::ClampToEdge;
    case aiTextureMapMode_Decal:
      return rendering::TextureRepeatMode::ClampToBorder;
  }

  return rendering::TextureRepeatMode::Unknown;
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
