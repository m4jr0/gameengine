// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_MODEL_RESOURCE_H_
#define COMET_COMET_RESOURCE_MODEL_RESOURCE_H_

#include "comet_precompile.h"

#include "glm/glm.hpp"

#include "comet/resource/resource.h"
#include "comet/resource/texture_resource.h"

namespace comet {
namespace resource {
namespace model {
struct ModelResourceDescr {
  // TODO(m4jr0): Add description.
  u8 empty{0};
};

struct TextureTuple {
  ResourceId texture_id{kInvalidResourceId};
  texture::TextureType type{texture::TextureType::Unknown};
};

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 tangent;
  glm::vec3 bitangent;
  glm::vec2 uv;
};

using Index = u32;

struct MeshResource : InternalResource {
  std::vector<Vertex> vertices;
  std::vector<Index> indices;
  std::vector<TextureTuple> textures;
};

struct ModelResource : Resource {
  static const ResourceTypeId kResourceTypeId;

  std::vector<MeshResource> meshes;
  ModelResourceDescr descr;
};

class ModelHandler : public ResourceHandler {
 public:
  ModelHandler() = default;
  ModelHandler(const ModelHandler&) = delete;
  ModelHandler(ModelHandler&&) = delete;
  ModelHandler& operator=(const ModelHandler&) = delete;
  ModelHandler& operator=(ModelHandler&&) = delete;
  ~ModelHandler() = default;

 protected:
  uindex GetMeshSize(const MeshResource& mesh) const;
  uindex GetModelSize(const ModelResource& model) const;

  ResourceFile Pack(const Resource& resource,
                    CompressionMode compression_mode) const override;
  std::unique_ptr<Resource> Unpack(const ResourceFile& file) const override;
};
}  // namespace model
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_MODEL_RESOURCE_H_
