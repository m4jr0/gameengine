// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "resource.h"

#include <fstream>

#include "comet/utils/file_system.h"

namespace comet {
namespace resource {
const Resource* ResourceCache::Set(std::unique_ptr<Resource> resource) {
  const auto it{cache_.emplace(resource->id, std::move(resource))};
  COMET_ASSERT(it.second, "Unable to save resource to cache!");
  return it.first->second.get();
}

const Resource* ResourceCache::Get(ResourceId resource_id) const {
  if (cache_.find(resource_id) == cache_.cend()) {
    return nullptr;
  }

  return cache_.at(resource_id).get();
}

ResourceId GenerateResourceIdFromPath(const std::string& resource_path) {
  return GenerateResourceIdFromPath(resource_path.c_str());
}

ResourceId GenerateResourceIdFromPath(const schar* resource_path) {
  return COMET_STRING_ID(resource_path);
}

void PackBytes(const u8* bytes, uindex bytes_size,
               CompressionMode compression_mode, std::vector<u8>* packed_bytes,
               uindex* packed_bytes_size) {
  switch (compression_mode) {
    case CompressionMode::Lz4: {
      utils::compression::CompressLz4(bytes, bytes_size, *packed_bytes);
      *packed_bytes_size = packed_bytes->size();
      break;
    }
    case CompressionMode::None: {
      std::memcpy(packed_bytes, bytes, bytes_size);
      *packed_bytes_size = bytes_size;
      break;
    }
    default: {
      throw std::runtime_error(
          "Unknown compression mode: " +
          static_cast<std::underlying_type_t<resource::CompressionMode>>(
              compression_mode));
    }
  }
}

void PackBytes(const std::vector<u8>& bytes, CompressionMode compression_mode,
               std::vector<u8>* packed_bytes, uindex* packed_bytes_size) {
  PackBytes(bytes.data(), bytes.size(), compression_mode, packed_bytes,
            packed_bytes_size);
}

void PackResourceData(const std::vector<u8>& data, ResourceFile& file) {
  file.data_size = data.size();
  PackBytes(data, file.compression_mode, &file.data, &file.packed_data_size);
}

std::vector<u8> UnpackBytes(CompressionMode compression_mode,
                            const u8* packed_bytes, uindex packed_bytes_size,
                            uindex decompressed_size) {
  std::vector<u8> data;

  switch (compression_mode) {
    case CompressionMode::Lz4: {
      utils::compression::DecompressLz4(packed_bytes, packed_bytes_size,
                                        decompressed_size, data);
      break;
    }
    case CompressionMode::None: {
      data.resize(decompressed_size);
      std::memcpy(data.data(), packed_bytes, decompressed_size);
      break;
    }
    default: {
      throw std::runtime_error(
          "Unknown compression mode: " +
          static_cast<std::underlying_type_t<resource::CompressionMode>>(
              compression_mode));
    }
  }

  return data;
}

std::vector<u8> UnpackBytes(CompressionMode compression_mode,
                            const std::vector<u8>& packed_bytes,
                            uindex decompressed_size) {
  return UnpackBytes(compression_mode, packed_bytes.data(), packed_bytes.size(),
                     decompressed_size);
}

std::vector<u8> UnpackResourceData(const ResourceFile& file) {
  return UnpackBytes(file.compression_mode, file.data, file.data_size);
}

bool SaveResourceFile(const std::string& path, const ResourceFile& file) {
  std::ofstream out_file;

  if (!utils::filesystem::OpenBinaryFileToWriteTo(path, out_file, false)) {
    COMET_LOG_RESOURCE_ERROR("Unable to write resource file: ", path);
    return false;
  }

  out_file.write(reinterpret_cast<const schar*>(&file.resource_type_id),
                 static_cast<std::streamsize>(sizeof(file.resource_type_id)));

  out_file.write(reinterpret_cast<const schar*>(&file.compression_mode),
                 static_cast<std::streamsize>(sizeof(file.compression_mode)));

  out_file.write(reinterpret_cast<const schar*>(&file.packed_descr_size),
                 static_cast<std::streamsize>(sizeof(file.packed_descr_size)));

  out_file.write(reinterpret_cast<const schar*>(&file.packed_data_size),
                 static_cast<std::streamsize>(sizeof(file.packed_data_size)));

  out_file.write(reinterpret_cast<const schar*>(&file.descr_size),
                 static_cast<std::streamsize>(sizeof(file.descr_size)));

  out_file.write(reinterpret_cast<const schar*>(&file.data_size),
                 static_cast<std::streamsize>(sizeof(file.data_size)));

  out_file.write(reinterpret_cast<const schar*>(file.descr.data()),
                 file.packed_descr_size);

  out_file.write(reinterpret_cast<const schar*>(file.data.data()),
                 file.packed_data_size);

  utils::filesystem::CloseFile(out_file);
  return true;
}

bool LoadResourceFile(const std::string& path, ResourceFile& file) {
  std::ifstream in_file;

  if (!utils::filesystem::OpenBinaryFileToReadFrom(path, in_file)) {
    COMET_LOG_RESOURCE_ERROR("Unable to open resource file: ", path);
    return false;
  }

  in_file.seekg(0);
  in_file.read(reinterpret_cast<schar*>(&file.resource_type_id),
               sizeof(ResourceId));

  in_file.read(reinterpret_cast<schar*>(&file.compression_mode),
               sizeof(file.compression_mode));

  in_file.read(reinterpret_cast<schar*>(&file.packed_descr_size),
               sizeof(file.packed_descr_size));

  in_file.read(reinterpret_cast<schar*>(&file.packed_data_size),
               sizeof(file.packed_data_size));

  in_file.read(reinterpret_cast<schar*>(&file.descr_size),
               sizeof(file.descr_size));

  in_file.read(reinterpret_cast<schar*>(&file.data_size),
               sizeof(file.data_size));

  file.descr.resize(file.packed_descr_size);
  in_file.read(reinterpret_cast<schar*>(file.descr.data()),
               file.packed_descr_size);

  file.data.resize(file.packed_data_size);
  in_file.read(reinterpret_cast<schar*>(file.data.data()),
               file.packed_data_size);

  return true;
}

const Resource* ResourceHandler::Load(std::string_view root_resource_path,
                                      const std::string& resource_path) {
  return Load(root_resource_path, resource_path.c_str());
}

const Resource* ResourceHandler::Load(std::string_view root_resource_path,
                                      const schar* resource_path) {
  return Load(root_resource_path, GenerateResourceIdFromPath(resource_path));
}

const Resource* ResourceHandler::Load(std::string_view root_resource_path,
                                      ResourceId resource_id) {
  if (resource_id == kDefaultResourceId) {
    return GetDefaultResource();
  }

  const auto* resource{Get(resource_id)};

  if (resource != nullptr) {
    return resource;
  }

  ResourceFile file{};

  const auto resource_abs_path = utils::filesystem::Append(
      root_resource_path, std::to_string(resource_id));

  const auto is_load{LoadResourceFile(resource_abs_path, file)};

  if (!is_load) {
    COMET_LOG_RESOURCE_ERROR("Unable to get resource with ID: ",
                             COMET_STRING_ID_LABEL(resource_id), ".");
    return GetDefaultResource();
  }

  return cache_.Set(Unpack(file));
}

const Resource* ResourceHandler::Get(ResourceId resource_id) {
  return cache_.Get(resource_id);
}

const Resource* ResourceHandler::GetDefaultResource() { return nullptr; }

ResourceFile ResourceHandler::GetResourceFile(
    const Resource& resource, CompressionMode compression_mode) const {
  return Pack(resource, compression_mode);
}
}  // namespace resource
}  // namespace comet
