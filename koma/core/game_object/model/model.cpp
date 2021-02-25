// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <core/locator/locator.hpp>
#include <core/render/texture/texture_loader.hpp>
#include <utils/file_system.hpp>
#include <utils/logger.hpp>
#include "mesh.hpp"

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include <debug_windows.hpp>
#endif  // _WIN32

namespace koma {
Model::Model(const std::string &model_path,
             std::shared_ptr<ShaderProgram> shader_program) {
  this->path_ = model_path;
  this->shader_program_ = shader_program;
}

void Model::Draw(std::shared_ptr<ShaderProgram> shader_program) {
  std::size_t meshes_number = this->meshes_.size();

  for (std::size_t index = 0; index < meshes_number; ++index) {
    this->meshes_[index].Draw(shader_program);
  }
}

void Model::LoadModel() {
  Assimp::Importer importer;

  const aiScene *scene = importer.ReadFile(
    this->path_,
    aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace
  );

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    Logger::Get(LOGGER_KOMA_CORE_GAME_OBJECT_MODEL_MODEL)->Error(
      "Assimp error: ", importer.GetErrorString()
    );

    return;
  }

  this->directory_ = filesystem::GetDirectoryPath(this->path_);

  this->LoadNode(scene->mRootNode, scene);
}

void Model::LoadNode(aiNode *current_node, const aiScene *scene) {
  for (std::size_t index = 0; index < current_node->mNumMeshes; ++index) {
    aiMesh *mesh = scene->mMeshes[current_node->mMeshes[index]];
    this->meshes_.push_back(this->LoadMesh(mesh, scene));
  }

  for (std::size_t index = 0; index < current_node->mNumChildren; ++index) {
    this->LoadNode(current_node->mChildren[index], scene);
  }
}

Mesh Model::LoadMesh(aiMesh *current_mesh, const aiScene *scene) {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  for (std::size_t index = 0; index < current_mesh->mNumVertices; ++index) {
    Vertex vertex;

    if (current_mesh->mVertices) {
      vertex.position = glm::vec3(
        current_mesh->mVertices[index].x,
        current_mesh->mVertices[index].y,
        current_mesh->mVertices[index].z
      );
    }

    if (current_mesh->mNormals) {
      vertex.normal = glm::vec3(
        current_mesh->mNormals[index].x,
        current_mesh->mNormals[index].y,
        current_mesh->mNormals[index].z
      );
    }

    if (current_mesh->mTangents) {
      vertex.tangent = glm::vec3(
        current_mesh->mTangents[index].x,
        current_mesh->mTangents[index].y,
        current_mesh->mTangents[index].z
      );
    }

    if (current_mesh->mBitangents) {
      vertex.bitangent = glm::vec3(
        current_mesh->mBitangents[index].x,
        current_mesh->mBitangents[index].y,
        current_mesh->mBitangents[index].z
      );
    }

    // Does our current mesh contain texture coordinates?
    if (current_mesh->mTextureCoords[0]) {
      vertex.texture_coordinates = glm::vec2(
        current_mesh->mTextureCoords[0][index].x,
        current_mesh->mTextureCoords[0][index].y
      );
    } else {
      vertex.texture_coordinates = glm::vec2(0.0f, 0.0f);
    }

    vertices.push_back(vertex);
  }

  for (std::size_t index = 0; index < current_mesh->mNumFaces; ++index) {
    aiFace face = current_mesh->mFaces[index];

    for (std::size_t face_index = 0; face_index < face.mNumIndices;
         ++face_index) {
      indices.push_back(face.mIndices[face_index]);
    }
  }

  if (current_mesh->mMaterialIndex >= 0) {
    aiMaterial *material = scene->mMaterials[current_mesh->mMaterialIndex];

    std::vector<Texture> diffuse_maps = this->LoadMaterialTextures(
      material, aiTextureType_DIFFUSE, "texture_diffuse"
    );

    textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

    std::vector<Texture> specular_maps = this->LoadMaterialTextures(
      material, aiTextureType_SPECULAR, "texture_specular"
    );

    textures.insert(
      textures.end(), specular_maps.begin(), specular_maps.end()
    );

    std::vector<Texture> normal_maps = this->LoadMaterialTextures(
      material, aiTextureType_HEIGHT, "texture_normal"
    );

    textures.insert(
      textures.end(), normal_maps.begin(), normal_maps.end()
    );

    std::vector<Texture> height_maps = this->LoadMaterialTextures(
      material, aiTextureType_AMBIENT, "texture_height"
    );

    textures.insert(
      textures.end(), height_maps.begin(), height_maps.end()
    );
  }

  return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::LoadMaterialTextures(aiMaterial *material,
                                                 aiTextureType texture_type,
                                                 std::string texture_type_name
                                                 ) {
  std::vector<Texture> textures;
  std::size_t texture_number = material->GetTextureCount(texture_type);

  for (std::size_t index = 0; index < texture_number; ++index) {
    aiString texture_path;
    material->GetTexture(texture_type, index, &texture_path);

    bool is_skip = false;
    std::size_t loaded_textures_number = this->loaded_textures_.size();

    for (std::size_t tl_index = 0; tl_index < loaded_textures_number;
         ++tl_index) {
      if (std::strcmp(
        this->loaded_textures_[tl_index].path.data(),
        texture_path.C_Str()
      ) == 0) {
        textures.push_back(this->loaded_textures_[tl_index]);
        is_skip = true;

        break;
      }
    }

    if (is_skip) continue;

    Texture texture;

    texture.id = Load2DTextureFromFile(
      this->directory_ + "/" + texture_path.C_Str()
    );

    texture.type = texture_type_name;
    texture.path = texture_path.C_Str();

    textures.push_back(texture);
    this->loaded_textures_.push_back(texture);
  }

  return textures;
}

void Model::Initialize() {
  this->Component::Initialize();

  if (!(this->transform_ = this->game_object_->GetComponent<Transform>())) {
    Logger::Get(LOGGER_KOMA_CORE_GAME_OBJECT_MODEL_MODEL)->Error(
      "A 'Transform' component is required when adding a Model component"
    );

    return;
  }

  this->LoadModel();
}

void Model::Update() {
  if (!this->shader_program_) return;

  this->shader_program_->Use();

  glm::mat4 mvp = Locator::main_camera()->GetMvp(
    this->transform_->GetTransformMatrix()
  );

  this->shader_program_->SetMatrix4("mvp", mvp);
  this->Draw(this->shader_program_);
}

void Model::Destroy() {
  for (auto texture : this->loaded_textures_) {
    glDeleteTextures(1, &texture.id);
  }
}
}  // namespace koma
