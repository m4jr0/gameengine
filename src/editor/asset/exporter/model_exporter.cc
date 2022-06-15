// Copyright 2022 m4jr0. All Rights Reserved.
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
bool ModelExporter::IsCompatible(const std::string& extension) {
  return extension == "obj";
}

std::vector<resource::ResourceFile> ModelExporter::GetResourceFiles(
    AssetDescr& asset_descr) {
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

  resource_files.emplace_back(
      Engine::Get().GetResourceManager().GetResourceFile(model,
                                                         compression_mode_));

  LoadMaterials(utils::filesystem::GetDirectoryPath(asset_descr.asset_abs_path),
                scene, resource_files);

  return resource_files;
}

void ModelExporter::LoadNode(resource::ModelResource& model,
                             const aiNode* current_node, const aiScene* scene) {
  for (uindex index{0}; index < current_node->mNumMeshes; ++index) {
    const auto* mesh{scene->mMeshes[current_node->mMeshes[index]]};
    LoadMesh(model, mesh, scene);
  }

  for (uindex index{0}; index < current_node->mNumChildren; ++index) {
    LoadNode(model, current_node->mChildren[index], scene);
  }
}

void ModelExporter::LoadMesh(resource::ModelResource& model,
                             const aiMesh* current_mesh, const aiScene* scene) {
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

  resource::MeshResource&& mesh_resource{};
  mesh_resource.resource_id = model.id;
  mesh_resource.internal_id = model.meshes.size();
  mesh_resource.vertices = std::move(vertices);
  mesh_resource.indices = std::move(indices);

  auto* raw_material{scene->mMaterials[current_mesh->mMaterialIndex]};
  // TODO(m4jr0): Handle better material IDs (names are easilly
  // duplicated...).
  mesh_resource.material_id =
      resource::GenerateMaterialId(raw_material->GetName().C_Str());

  model.meshes.push_back(std::move(mesh_resource));
}

void ModelExporter::LoadMaterialTextures(const std::string& resource_path,
                                         resource::MaterialResource& material,
                                         aiMaterial* raw_material,
                                         aiTextureType raw_texture_type) {
  std::vector<resource::TextureTuple> textures;
  const auto texture_count{raw_material->GetTextureCount(raw_texture_type)};

  if (texture_count == 0) {
    return;
  }

  rendering::TextureType texture_type{GetTextureType(raw_texture_type)};

  for (uindex i{0}; i < texture_count; ++i) {
    aiString texture_path;
    raw_material->GetTexture(raw_texture_type, i, &texture_path);
    material.texture_tuples.emplace_back(resource::TextureTuple{
        resource::GenerateResourceIdFromPath(
            utils::filesystem::Append(resource_path, texture_path.C_Str())),
        texture_type});
  }
}

void ModelExporter::LoadMaterials(
    const std::string& directory_path, const aiScene* scene,
    std::vector<resource::ResourceFile>& resource_files) {
  const auto resource_path{
      utils::filesystem::GetRelativePath(directory_path, root_asset_path_)};
  constexpr std::array<aiTextureType, 4> exported_texture_types{
      {aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_HEIGHT,
       aiTextureType_AMBIENT}};

  for (uindex i{0}; i < scene->mNumMaterials; ++i) {
    const auto raw_material{scene->mMaterials[i]};
    resource::MaterialResource material{};

    COMET_LOG_GLOBAL_DEBUG("Material ", raw_material->GetName().C_Str(),
                           " found.");

    for (auto raw_texture_type : exported_texture_types) {
      LoadMaterialTextures(resource_path, material, raw_material,
                           raw_texture_type);
    }

    // TODO(m4jr0): Handle better material IDs (names are easilly
    // duplicated...).
    material.id = resource::GenerateMaterialId(raw_material->GetName().C_Str());
    material.type_id = resource::MaterialResource::kResourceTypeId;

    resource_files.emplace_back(
        Engine::Get().GetResourceManager().GetResourceFile(material,
                                                           compression_mode_));
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
}  // namespace asset
}  // namespace editor
}  // namespace comet
