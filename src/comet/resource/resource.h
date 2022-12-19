// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_RESOURCE_H_
#define COMET_COMET_RESOURCE_RESOURCE_H_

#include "comet_precompile.h"

#include "comet/utils/compression.h"
#include "comet/utils/file_system.h"

namespace comet {
namespace resource {
using ResourceId = stringid::StringId;
constexpr auto kInvalidResourceId{static_cast<ResourceId>(-1)};
using ResourceTypeId = stringid::StringId;
constexpr auto kDefaultResourceId{0};
constexpr auto kInvalidResourceTypeId{static_cast<ResourceTypeId>(-1)};
using ResourcePath = std::string;

enum class CompressionMode : u8 { None = 0, Lz4 };

enum class ResourceLifeSpan : u8 {
  Unknown = 0,
  Frame,
  DoubleFrame,
  Level,
  Global
};

struct ResourceFile {
  ResourceId resource_id{kInvalidResourceId};
  ResourceId resource_type_id{kInvalidResourceTypeId};
  CompressionMode compression_mode{CompressionMode::None};
  uindex descr_size{0};
  uindex data_size{0};
  uindex packed_descr_size{0};
  uindex packed_data_size{0};
  std::vector<u8> descr{};
  std::vector<u8> data{};
};

struct Resource {
  ResourceId id{kInvalidResourceId};
  ResourceId type_id{kInvalidResourceTypeId};

  virtual ~Resource() = default;
};

struct InternalResource {
  ResourceId resource_id{kInvalidResourceId};
  ResourceId internal_id{kInvalidResourceId};
};

class ResourceCache {
 public:
  ResourceCache() = default;
  ResourceCache(const ResourceCache&) = delete;
  ResourceCache(ResourceCache&&) = delete;
  ResourceCache& operator=(const ResourceCache&) = delete;
  ResourceCache& operator=(ResourceCache&&) = delete;
  ~ResourceCache() = default;

  const Resource* Set(std::unique_ptr<Resource> resource);
  const Resource* Get(ResourceId resource_id) const;

 private:
  std::unordered_map<ResourceId, std::unique_ptr<Resource>> cache_{};
};

ResourceId GenerateResourceIdFromPath(const std::string& resource_path);
ResourceId GenerateResourceIdFromPath(const schar* resource_path);
void PackBytes(const u8* bytes, uindex bytes_size,
               CompressionMode compression_mode, std::vector<u8>* packed_bytes,
               uindex* packed_bytes_size);
void PackBytes(const std::vector<u8>& bytes, CompressionMode compression_mode,
               std::vector<u8>* packed_bytes, uindex* packed_bytes_size);
void PackResourceData(const std::vector<u8>& data, ResourceFile& file);

template <typename ResourceDescrType>
void PackPodResourceDescr(const ResourceDescrType& descr, ResourceFile& file) {
  file.descr_size = sizeof(descr);
  PackBytes(reinterpret_cast<const u8*>(&descr), file.descr_size,
            file.compression_mode, &file.descr, &file.packed_descr_size);
}

std::vector<u8> UnpackBytes(CompressionMode compression_mode,
                            const u8* packed_bytes, uindex packed_bytes_size,
                            uindex decompressed_size);
std::vector<u8> UnpackBytes(CompressionMode compression_mode,
                            const std::vector<u8>& packed_bytes,
                            uindex decompressed_size);

std::vector<u8> UnpackResourceData(const ResourceFile& file);

template <typename ResourceDescrType>
ResourceDescrType UnpackPodResourceDescr(const ResourceFile& file) {
  const auto raw_descr{
      UnpackBytes(file.compression_mode, file.descr, file.descr_size)};
  return *reinterpret_cast<const ResourceDescrType*>(raw_descr.data());
}

bool SaveResourceFile(const std::string& path, const ResourceFile& file);
bool LoadResourceFile(const std::string& path, ResourceFile& file);

class ResourceHandler {
 public:
  ResourceHandler() = default;
  ResourceHandler(const ResourceHandler&) = delete;
  ResourceHandler(ResourceHandler&&) = delete;
  ResourceHandler& operator=(const ResourceHandler&) = delete;
  ResourceHandler& operator=(ResourceHandler&&) = delete;
  virtual ~ResourceHandler() = default;

  const Resource* Load(std::string_view root_resource_path,
                       const std::string& resource_path);
  const Resource* Load(std::string_view root_resource_path,
                       const schar* resource_path);
  const Resource* Load(std::string_view root_resource_path,
                       ResourceId resource_id);
  virtual const Resource* Get(ResourceId resource_id);
  virtual const Resource* GetDefaultResource();
  ResourceFile GetResourceFile(const Resource& resource,
                               CompressionMode compression_mode) const;

 protected:
  virtual ResourceFile Pack(const Resource& resource,
                            CompressionMode compression_mode) const = 0;
  virtual std::unique_ptr<Resource> Unpack(const ResourceFile& file) const = 0;

  ResourceCache cache_{};
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_RESOURCE_H_
