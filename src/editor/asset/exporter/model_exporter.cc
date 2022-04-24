// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "model_exporter.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "comet/core/engine.h"
#include "comet/resource/texture_resource.h"
#include "comet/utils/file_system.h"

namespace comet {
namespace editor {
namespace asset {
namespace model {
bool ModelExporter::IsCompatible(const std::string& extension) {
  return extension == "obj";
}

bool ModelExporter::AttachResourceToAssetDescr(AssetDescr& asset_descr) {
  Assimp::Importer importer;

  const auto* scene{importer.ReadFile(
      asset_descr.asset_abs_path,
      aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace)};

  if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      scene->mRootNode == nullptr) {
    COMET_LOG_RESOURCE_ERROR("Assimp error: ", importer.GetErrorString());
    return false;
  }

  const auto directory_path{
      utils::filesystem::GetDirectoryPath(asset_descr.asset_abs_path)};

  resource::model::ModelResource model;
  model.id = asset_descr.resource_id;
  model.type_id = resource::model::ModelResource::kResourceTypeId;
  LoadNode(directory_path, model, scene->mRootNode, scene);
  asset_descr.resource = Engine::Get().GetResourceManager().GetResourceFile(
      model, compression_mode_);

  return true;
}

void ModelExporter::LoadNode(const std::string& directory_path,
                             resource::model::ModelResource& model,
                             const aiNode* current_node, const aiScene* scene) {
  for (uindex index{0}; index < current_node->mNumMeshes; ++index) {
    const auto* mesh{scene->mMeshes[current_node->mMeshes[index]]};
    LoadMesh(directory_path, model, mesh, scene);
  }

  for (uindex index{0}; index < current_node->mNumChildren; ++index) {
    LoadNode(directory_path, model, current_node->mChildren[index], scene);
  }
}

void ModelExporter::LoadMesh(const std::string& directory_path,
                             resource::model::ModelResource& model,
                             const aiMesh* current_mesh, const aiScene* scene) {
  std::vector<resource::model::Vertex> vertices;
  std::vector<resource::model::Index> indices;
  std::vector<resource::model::TextureTuple> textures;

  for (uindex index{0}; index < current_mesh->mNumVertices; ++index) {
    resource::model::Vertex vertex{};

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

  auto resource_path{
      utils::filesystem::GetRelativePath(directory_path, root_asset_path_)};

  if (current_mesh->mMaterialIndex >= 0) {
    const auto material{scene->mMaterials[current_mesh->mMaterialIndex]};

    const auto diffuse_maps{
        LoadMaterialTextures(resource_path, material, aiTextureType_DIFFUSE)};

    textures.insert(textures.cend(), diffuse_maps.cbegin(),
                    diffuse_maps.cend());

    const auto specular_maps{
        LoadMaterialTextures(resource_path, material, aiTextureType_SPECULAR)};

    textures.insert(textures.cend(), specular_maps.cbegin(),
                    specular_maps.cend());

    const auto normal_maps{
        LoadMaterialTextures(resource_path, material, aiTextureType_HEIGHT)};

    textures.insert(textures.cend(), normal_maps.cbegin(), normal_maps.cend());

    const auto height_maps{
        LoadMaterialTextures(resource_path, material, aiTextureType_AMBIENT)};

    textures.insert(textures.cend(), height_maps.cbegin(), height_maps.cend());
  }

  model.meshes.push_back({model.id, vertices, indices, textures});
}

std::vector<resource::model::TextureTuple> ModelExporter::LoadMaterialTextures(
    const std::string& resource_path, aiMaterial* material,
    aiTextureType assimp_texture_type) {
  std::vector<resource::model::TextureTuple> textures;
  const auto texture_count{material->GetTextureCount(assimp_texture_type)};
  resource::texture::TextureType texture_type;

  switch (assimp_texture_type) {
    case aiTextureType_DIFFUSE: {
      texture_type = resource::texture::TextureType::Diffuse;
      break;
    }
    case aiTextureType_SPECULAR: {
      texture_type = resource::texture::TextureType::Specular;
      break;
    }
    case aiTextureType_HEIGHT: {
      texture_type = resource::texture::TextureType::Height;
      break;
    }
    case aiTextureType_AMBIENT: {
      texture_type = resource::texture::TextureType::Ambient;
      break;
    }
    default: {
      texture_type = resource::texture::TextureType::Unknown;
      break;
    }
  }

  for (uindex i{0}; i < texture_count; ++i) {
    aiString texture_path;
    material->GetTexture(assimp_texture_type, i, &texture_path);
    textures.push_back({resource::GenerateResourceId(utils::filesystem::Append(
                            resource_path, texture_path.C_Str())),
                        texture_type});
  }

  return textures;
}
}  // namespace model
}  // namespace asset
}  // namespace editor
}  // namespace comet
