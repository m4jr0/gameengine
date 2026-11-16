// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/resource/model/model_resource_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/memory_utils.h"
#include "comet/core/container/array.h"
#include "comet/data/geometry/mesh.h"
#include "comet/data/geometry/skeleton.h"

namespace comet {
namespace resource {
namespace internal {
template <typename MeshResourceT>
void PackMeshResource(const MeshResourceT& mesh, u8* buffer, usize& cursor) {
  const auto vertex_count{mesh.vertices.GetSize()};
  const auto index_count{mesh.indices.GetSize()};

  memory::CopyMemory(&buffer[cursor], &mesh.resource_id, sizeof(RawResourceId));
  cursor += sizeof(RawResourceId);

  memory::CopyMemory(&buffer[cursor], &mesh.internal_id, sizeof(RawResourceId));
  cursor += sizeof(RawResourceId);

  memory::CopyMemory(&buffer[cursor], &mesh.type, sizeof(geometry::MeshType));
  cursor += sizeof(geometry::MeshType);

  memory::CopyMemory(&buffer[cursor], &mesh.material_resource_id,
                     sizeof(MaterialResourceId));
  cursor += sizeof(MaterialResourceId);

  memory::CopyMemory(&buffer[cursor], &mesh.transform, sizeof(math::Mat4));
  cursor += sizeof(math::Mat4);

  memory::CopyMemory(&buffer[cursor], &mesh.local_center, sizeof(math::Vec3));
  cursor += sizeof(math::Vec3);

  memory::CopyMemory(&buffer[cursor], &mesh.local_max_extents,
                     sizeof(math::Vec3));
  cursor += sizeof(math::Vec3);

  memory::CopyMemory(&buffer[cursor], &mesh.parent_id, sizeof(RawResourceId));
  cursor += sizeof(RawResourceId);

  memory::CopyMemory(&buffer[cursor], &vertex_count, sizeof(vertex_count));
  cursor += sizeof(vertex_count);

  if (vertex_count > 0) {
    const auto vertex_total_size{sizeof(geometry::SkinnedVertex) *
                                 vertex_count};
    memory::CopyMemory(&buffer[cursor], mesh.vertices.GetData(),
                       vertex_total_size);
    cursor += vertex_total_size;
  }

  memory::CopyMemory(&buffer[cursor], &index_count, sizeof(index_count));
  cursor += sizeof(index_count);

  if (index_count > 0) {
    const auto index_total_size{sizeof(geometry::Index) * index_count};
    memory::CopyMemory(&buffer[cursor], mesh.indices.GetData(),
                       index_total_size);
    cursor += index_total_size;
  }
}


}  // namespace internal

StaticModelResourceHandler::StaticModelResourceHandler(
    const ResourceHandlerDescr& descr)
    : Base{descr} {}

ResourceFile StaticModelResourceHandler::Pack(const StaticModelResource& resource,
    CompressionMode compression_mode) {
  return PackStaticModelResource(resource, compression_mode, byte_allocator_);
}

void StaticModelResourceHandler::Unpack(const ResourceFile& file,
    ResourceLifeSpan life_span, StaticModelResource* resource) {
  UnpackStaticModelResource(file, byte_allocator_, ResolveAllocator(byte_allocator_, life_span), resource);
  resource->life_span = life_span;
}

SkeletalModelResourceHandler::SkeletalModelResourceHandler(
    const ResourceHandlerDescr& descr)
    : Base{descr} {}

ResourceFile SkeletalModelResourceHandler::Pack(const SkeletalModelResource& resource,
    CompressionMode compression_mode) {
  return PackSkeletalModelResource(resource, compression_mode, byte_allocator_);
}

void SkeletalModelResourceHandler::Unpack(const ResourceFile& file,
    ResourceLifeSpan life_span, SkeletalModelResource* resource) {
  UnpackSkeletalModelResource(file, byte_allocator_, ResolveAllocator(byte_allocator_, life_span), resource);
  resource->life_span = life_span;
}

SkeletonResourceHandler::SkeletonResourceHandler(
    const ResourceHandlerDescr& descr)
    : Base{descr} {}

ResourceFile SkeletonResourceHandler::Pack(const SkeletonResource& resource,
    CompressionMode compression_mode) {
  return PackSkeletonResource(resource, compression_mode, byte_allocator_);
}

void SkeletonResourceHandler::Unpack(const ResourceFile& file,
    ResourceLifeSpan life_span, SkeletonResource* resource) {
  UnpackSkeletonResource(file, byte_allocator_, ResolveAllocator(byte_allocator_, life_span), resource);
  resource->life_span = life_span;
}
}  // namespace resource
}  // namespace comet