// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_MODEL_RESOURCE_H_
#define COMET_COMET_RESOURCE_MODEL_RESOURCE_H_

#include "comet_precompile.h"

#include "comet/math/bounding_volume.h"
#include "comet/math/matrix.h"
#include "comet/math/vector.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/material_resource.h"
#include "comet/resource/resource.h"

namespace comet {
namespace resource {
struct ModelResourceDescr {
  // TODO(m4jr0): Add description.
  u8 empty{0};
};

struct MeshResource : InternalResource {
  ResourceId material_id{kInvalidResourceId};
  math::Mat4 transform{1.0f};
  math::Vec3 local_center{0.0f};
  math::Vec3 local_max_extents{0.0f};
  ResourceId parent_id{kInvalidResourceId};
  std::vector<rendering::Vertex> vertices{};
  std::vector<rendering::Index> indices{};
};

struct ModelResource : Resource {
  static const ResourceTypeId kResourceTypeId;

  std::vector<MeshResource> meshes{};
  ModelResourceDescr descr{};
};

class ModelHandler : public ResourceHandler {
 public:
  ModelHandler() = default;
  ModelHandler(const ModelHandler&) = delete;
  ModelHandler(ModelHandler&&) = delete;
  ModelHandler& operator=(const ModelHandler&) = delete;
  ModelHandler& operator=(ModelHandler&&) = delete;
  virtual ~ModelHandler() = default;

 protected:
  uindex GetMeshSize(const MeshResource& mesh) const;
  uindex GetModelSize(const ModelResource& model) const;

  ResourceFile Pack(const Resource& resource,
                    CompressionMode compression_mode) const override;
  std::unique_ptr<Resource> Unpack(const ResourceFile& file) const override;
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_MODEL_RESOURCE_H_
