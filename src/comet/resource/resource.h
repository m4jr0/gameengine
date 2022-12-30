// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_RESOURCE_H_
#define COMET_COMET_RESOURCE_RESOURCE_H_

#include "comet_precompile.h"

#include "comet/utils/compression.h"
#include "comet/utils/file_system.h"
#include "comet/utils/hash.h"

namespace comet {
namespace resource {
using ResourceId = stringid::StringId;
constexpr auto kInvalidResourceId{static_cast<ResourceId>(-1)};
using ResourceTypeId = stringid::StringId;
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
  std::vector<char> descr{};
  std::vector<char> data{};
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

template <typename ResourcePath>
ResourceId GenerateResourceIdFromPath(ResourcePath&& resource_path) {
  return utils::hash::HashCrC32(std::forward<ResourcePath>(resource_path));
}

template <typename ResourceTypeName>
ResourceId GenerateResourceTypeId(ResourceTypeName&& resource_type_name) {
  return utils::hash::HashCrC32(
      std::forward<ResourceTypeName>(resource_type_name));
}

template <typename ResourceDescrType>
void PackResourceDescr(const ResourceDescrType& descr, ResourceFile& file) {
  file.descr_size = sizeof(descr);

  switch (file.compression_mode) {
    case CompressionMode::Lz4: {
      utils::compression::CompressLz4(
          descr, static_cast<uindex>(file.descr_size), file.descr);
      file.packed_descr_size =
          static_cast<uindex>(sizeof(char) * file.descr.size());
      break;
    }
    case CompressionMode::None: {
      const auto* descr_buffer{reinterpret_cast<const char*>(&descr)};
      file.descr = {descr_buffer, descr_buffer + file.descr_size};
      file.packed_descr_size = file.descr_size;
      break;
    }
    default: {
      throw std::runtime_error(
          "Unknown compression mode: " +
          std::to_string(
              static_cast<std::underlying_type_t<resource::CompressionMode>>(
                  file.compression_mode)));
    }
  }
}

void PackResourceData(const std::vector<u8>& data, ResourceFile& file);

template <typename ResourceDescrType>
ResourceDescrType UnpackResourceDescr(const ResourceFile& file) {
  ResourceDescrType descr{};

  switch (file.compression_mode) {
    case CompressionMode::Lz4: {
      utils::compression::DecompressLz4(file.descr, file.descr_size, descr);
      break;
    }
    case CompressionMode::None: {
      descr = *reinterpret_cast<const ResourceDescrType*>(file.descr.data());
      break;
    }
    default: {
      throw std::runtime_error(
          "Unknown compression mode: " +
          std::to_string(
              static_cast<std::underlying_type_t<resource::CompressionMode>>(
                  file.compression_mode)));
    }
  }

  return descr;
}

std::vector<u8> UnpackResourceData(const ResourceFile& file);
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

  template <typename AbsolutePath, typename ResourcePath>
  const std::unique_ptr<Resource> Load(AbsolutePath&& root_resource_path,
                                       ResourcePath&& resource_path) const {
    const auto resource_id = GenerateResourceIdFromPath(resource_path);
    ResourceFile file{};

    const auto resource_abs_path = utils::filesystem::Append(
        std::forward<AbsolutePath>(root_resource_path),
        std::forward<ResourcePath>(resource_path));

    const auto result{LoadResourceFile(resource_abs_path, file)};

    COMET_ASSERT(result, "Unable to get resource at path: ", resource_path,
                 ".");

    return Unpack(file);
  }

  ResourceFile GetResourceFile(const Resource& resource,
                               CompressionMode compression_mode) const;

 protected:
  virtual ResourceFile Pack(const Resource& resource,
                            CompressionMode compression_mode) const = 0;
  virtual std::unique_ptr<Resource> Unpack(const ResourceFile& file) const = 0;
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_RESOURCE_H_
