// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_MODEL_RESOURCE_H_
#define COMET_COMET_RESOURCE_MODEL_RESOURCE_H_

#include <memory>

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type/array.h"
#include "comet/geometry/geometry_common.h"
#include "comet/math/bounding_volume.h"
#include "comet/math/matrix.h"
#include "comet/math/vector.h"
#include "comet/resource/material_resource.h"
#include "comet/resource/resource.h"

namespace comet {
namespace resource {
struct MeshResource : InternalResource {
  geometry::MeshType type{geometry::MeshType::Unknown};
  ResourceId material_id{kInvalidResourceId};
  math::Mat4 transform{1.0f};
  math::Vec3 local_center{0.0f};
  math::Vec3 local_max_extents{0.0f};
  ResourceId parent_id{kInvalidResourceId};
  Array<geometry::Index> indices{};
};

struct StaticMeshResource : MeshResource {
  Array<geometry::Vertex> vertices{};
};

struct SkinnedMeshResource : MeshResource {
  Array<geometry::SkinnedVertex> vertices{};
};

struct StaticModelResourceDescr {
  // TODO(m4jr0): Add description.
  u8 empty{0};
};

struct StaticModelResource : Resource {
  static const ResourceTypeId kResourceTypeId;

  StaticModelResourceDescr descr{};
  Array<StaticMeshResource> meshes{};
};

struct SkeletalModelResourceDescr {
  // TODO(m4jr0): Add description.
  u8 empty{0};
};

struct SkeletalModelResource : Resource {
  static const ResourceTypeId kResourceTypeId;

  SkeletalModelResourceDescr descr{};
  Array<SkinnedMeshResource> meshes{};
};

class StaticModelHandler : public ResourceHandler {
 public:
  StaticModelHandler() = default;
  StaticModelHandler(const StaticModelHandler&) = delete;
  StaticModelHandler(StaticModelHandler&&) = delete;
  StaticModelHandler& operator=(const StaticModelHandler&) = delete;
  StaticModelHandler& operator=(StaticModelHandler&&) = delete;
  virtual ~StaticModelHandler() = default;

 protected:
  usize GetMeshSize(const StaticMeshResource& mesh) const;
  usize GetModelSize(const StaticModelResource& model) const;

  ResourceFile Pack(memory::Allocator& allocator, const Resource& resource,
                    CompressionMode compression_mode) const override;
  std::unique_ptr<Resource> Unpack(memory::Allocator& allocator,
                                   const ResourceFile& file) const override;
};

class SkinnedModelHandler : public ResourceHandler {
 public:
  SkinnedModelHandler() = default;
  SkinnedModelHandler(const SkinnedModelHandler&) = delete;
  SkinnedModelHandler(SkinnedModelHandler&&) = delete;
  SkinnedModelHandler& operator=(const SkinnedModelHandler&) = delete;
  SkinnedModelHandler& operator=(SkinnedModelHandler&&) = delete;
  virtual ~SkinnedModelHandler() = default;

 protected:
  usize GetMeshSize(const SkinnedMeshResource& mesh) const;
  usize GetModelSize(const SkeletalModelResource& model) const;

  ResourceFile Pack(memory::Allocator& allocator, const Resource& resource,
                    CompressionMode compression_mode) const override;
  std::unique_ptr<Resource> Unpack(memory::Allocator& allocator,
                                   const ResourceFile& file) const override;
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_MODEL_RESOURCE_H_
