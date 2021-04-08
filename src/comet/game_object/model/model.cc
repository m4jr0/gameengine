// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "model.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "comet/core/engine.h"
#include "comet/rendering/texture/texture_loader.h"
#include "comet/utils/file_system.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
Model::Model(const std::string &model_path,
             std::shared_ptr<ShaderProgram> shader_program) {
  path_ = model_path;
  shader_program_ = shader_program;
}

void Model::Initialize() {
  Component::Initialize();

  if ((transform_ = game_object_->GetComponent<Transform>()) == nullptr) {
    Logger::Get(LoggerType::GameObject)
        ->Error(
            "A 'Transform' component is required when adding a Model "
            "component");

    return;
  }

  LoadModel();
}

void Model::Destroy() {
  for (const auto &texture : loaded_textures_) {
    glDeleteTextures(1, &texture.id);
  }
}

void Model::Update() {
  if (shader_program_ == nullptr) return;

  shader_program_->Use();

  const auto mvp =
      Engine::engine()->main_camera()->GetMvp(transform_->GetTransformMatrix());

  shader_program_->SetMatrix4("mvp", mvp);
  Draw(shader_program_);
}

void Model::Draw(std::shared_ptr<ShaderProgram> shader_program) {
  const auto meshes_number = meshes_.size();

  for (std::size_t index = 0; index < meshes_number; ++index) {
    meshes_[index].Draw(shader_program);
  }
}

void Model::LoadModel() {
  Assimp::Importer importer;

  const auto *scene =
      importer.ReadFile(path_, aiProcess_Triangulate | aiProcess_FlipUVs |
                                   aiProcess_CalcTangentSpace);

  if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      scene->mRootNode == nullptr) {
    Logger::Get(LoggerType::GameObject)
        ->Error("Assimp error: ", importer.GetErrorString());

    return;
  }

  directory_ = filesystem::GetDirectoryPath(path_);
  LoadNode(scene->mRootNode, scene);
}

void Model::LoadNode(const aiNode *current_node, const aiScene *scene) {
  for (std::size_t index = 0; index < current_node->mNumMeshes; ++index) {
    const auto *mesh = scene->mMeshes[current_node->mMeshes[index]];
    meshes_.push_back(LoadMesh(mesh, scene));
  }

  for (std::size_t index = 0; index < current_node->mNumChildren; ++index) {
    LoadNode(current_node->mChildren[index], scene);
  }
}

Mesh Model::LoadMesh(const aiMesh *current_mesh, const aiScene *scene) {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  for (std::size_t index = 0; index < current_mesh->mNumVertices; ++index) {
    Vertex vertex{};

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
      vertex.texture_coordinates =
          glm::vec2(current_mesh->mTextureCoords[0][index].x,
                    current_mesh->mTextureCoords[0][index].y);
    } else {
      vertex.texture_coordinates = glm::vec2(0.0f, 0.0f);
    }

    vertices.push_back(vertex);
  }

  for (std::size_t index = 0; index < current_mesh->mNumFaces; ++index) {
    const auto &face = current_mesh->mFaces[index];

    for (std::size_t face_index = 0; face_index < face.mNumIndices;
         ++face_index) {
      indices.push_back(face.mIndices[face_index]);
    }
  }

  if (current_mesh->mMaterialIndex >= 0) {
    const auto material = scene->mMaterials[current_mesh->mMaterialIndex];

    const auto diffuse_maps = LoadMaterialTextures(
        material, aiTextureType_DIFFUSE, "texture_diffuse");

    textures.insert(textures.cend(), diffuse_maps.cbegin(),
                    diffuse_maps.cend());

    const auto specular_maps = LoadMaterialTextures(
        material, aiTextureType_SPECULAR, "texture_specular");

    textures.insert(textures.cend(), specular_maps.cbegin(),
                    specular_maps.cend());

    const auto normal_maps =
        LoadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");

    textures.insert(textures.cend(), normal_maps.cbegin(), normal_maps.cend());

    const auto height_maps =
        LoadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");

    textures.insert(textures.cend(), height_maps.cbegin(), height_maps.cend());
  }

  return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::LoadMaterialTextures(
    aiMaterial *material, aiTextureType texture_type,
    const std::string &texture_type_name) {
  std::vector<Texture> textures;
  const auto texture_number = material->GetTextureCount(texture_type);

  for (std::size_t index = 0; index < texture_number; ++index) {
    aiString texture_path;
    material->GetTexture(texture_type, index, &texture_path);

    auto is_skip = false;
    const auto loaded_textures_number = loaded_textures_.size();

    for (std::size_t tl_index = 0; tl_index < loaded_textures_number;
         ++tl_index) {
      if (std::strcmp(loaded_textures_[tl_index].path.data(),
                      texture_path.C_Str()) == 0) {
        textures.push_back(loaded_textures_[tl_index]);
        is_skip = true;

        break;
      }
    }

    if (is_skip) continue;

    Texture texture;
    texture.id = Load2DTextureFromFile(directory_ + "/" + texture_path.C_Str());
    texture.type = texture_type_name;
    texture.path = texture_path.C_Str();

    textures.push_back(texture);
    loaded_textures_.push_back(texture);
  }

  return textures;
}
}  // namespace comet
