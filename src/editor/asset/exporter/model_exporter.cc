// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "model_exporter.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "comet/core/engine.h"
#include "comet/rendering/rendering_common.h"
#include "comet/utils/file_system.h"

namespace comet {
namespace editor {
namespace asset {
bool ModelExporter::IsCompatible(std::string_view extension) const {
  return extension == "obj";
}

std::vector<resource::ResourceFile> ModelExporter::GetResourceFiles(
    AssetDescr& asset_descr) const {
  std::vector<resource::ResourceFile> resource_files{};
  Assimp::Importer importer;

  const auto* scene{importer.ReadFile(
      asset_descr.asset_abs_path,
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

  resource_files.push_back(Engine::Get().GetResourceManager().GetResourceFile(
      model, compression_mode_));

  LoadMaterials(utils::filesystem::GetDirectoryPath(asset_descr.asset_abs_path),
                scene, resource_files);

  return resource_files;
}

void ModelExporter::LoadNode(resource::ModelResource& model,
                             const aiNode* current_node,
                             const aiScene* scene) const {
  for (uindex index{0}; index < current_node->mNumMeshes; ++index) {
    const auto* mesh{scene->mMeshes[current_node->mMeshes[index]]};
    LoadMesh(model, mesh, scene);
  }

  for (uindex index{0}; index < current_node->mNumChildren; ++index) {
    LoadNode(model, current_node->mChildren[index], scene);
  }
}

void ModelExporter::LoadMesh(resource::ModelResource& model,
                             const aiMesh* current_mesh,
                             const aiScene* scene) const {
  std::vector<rendering::Vertex> vertices;
  std::vector<rendering::Index> indices;

  for (uindex index{0}; index < current_mesh->mNumVertices; ++index) {
    rendering::Vertex vertex{};

    if (current_mesh->mVertices) {
      vertex.position = glm::vec3(current_mesh->mVertices[index].x,
                                  current_mesh->mVertices[index].y,
                                  current_mesh->mVertices[index].z);
    }

    if (current_mesh->mNormals) {
      vertex.normal = glm::vec3(current_mesh->mNormals[index].x,
                                current_mesh->mNormals[index].y,
                                current_mesh->mNormals[index].z);
    }

    if (current_mesh->mTangents) {
      vertex.tangent = glm::vec3(current_mesh->mTangents[index].x,
                                 current_mesh->mTangents[index].y,
                                 current_mesh->mTangents[index].z);
    }

    if (current_mesh->mBitangents) {
      vertex.bitangent = glm::vec3(current_mesh->mBitangents[index].x,
                                   current_mesh->mBitangents[index].y,
                                   current_mesh->mBitangents[index].z);
    }

    // Does our current mesh contain texture coordinates?
    if (current_mesh->mTextureCoords[0]) {
      vertex.uv = glm::vec2(current_mesh->mTextureCoords[0][index].x,
                            current_mesh->mTextureCoords[0][index].y);
    } else {
      vertex.uv = glm::vec2(0.0f, 0.0f);
    }

    vertices.push_back(vertex);
  }

  for (uindex index{0}; index < current_mesh->mNumFaces; ++index) {
    const auto& face{current_mesh->mFaces[index]};

    for (uindex face_index{0}; face_index < face.mNumIndices; ++face_index) {
      indices.push_back(face.mIndices[face_index]);
    }
  }

  resource::MeshResource mesh_resource{};
  mesh_resource.resource_id = model.id;
  mesh_resource.internal_id = model.meshes.size();
  mesh_resource.vertices = std::move(vertices);
  mesh_resource.indices = std::move(indices);

  auto* raw_material{scene->mMaterials[current_mesh->mMaterialIndex]};
  mesh_resource.material_id =
      resource::GenerateMaterialId(raw_material->GetName().C_Str());

  model.meshes.push_back(std::move(mesh_resource));
}

void ModelExporter::LoadMaterialTextures(std::string_view resource_path,
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

  aiString texture_path;
  raw_material->GetTexture(raw_texture_type, 0, &texture_path);

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
    case rendering::TextureType::Height:
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

  map->texture_id = resource::GenerateResourceIdFromPath(
      utils::filesystem::Append(resource_path, texture_path.C_Str()));
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

void ModelExporter::LoadMaterials(
    std::string_view directory_path, const aiScene* scene,
    std::vector<resource::ResourceFile>& resource_files) const {
  const auto resource_path{
      utils::filesystem::GetRelativePath(directory_path, root_asset_path_)};
  constexpr std::array<aiTextureType, 4> exported_texture_types{
      {aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_HEIGHT,
       aiTextureType_AMBIENT}};

  for (uindex i{0}; i < scene->mNumMaterials; ++i) {
    const auto raw_material{scene->mMaterials[i]};

    COMET_LOG_GLOBAL_DEBUG("Material \"", raw_material->GetName().C_Str(),
                           "\" found.");

    resource::MaterialResource material{};
    material.descr.shader_id =
        resource::GenerateResourceIdFromPath("shaders/default_shader.cshader");

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

    material.descr.diffuse_color = glm::vec4(color.r, color.b, color.g, 1.0f);

    for (auto raw_texture_type : exported_texture_types) {
      LoadMaterialTextures(resource_path, material, raw_material,
                           raw_texture_type);
    }

    material.id = resource::GenerateMaterialId(raw_material->GetName().C_Str());
    material.type_id = resource::MaterialResource::kResourceTypeId;

    resource_files.push_back(Engine::Get().GetResourceManager().GetResourceFile(
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
      return rendering::TextureType::Height;
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
