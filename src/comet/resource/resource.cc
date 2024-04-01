// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "resource.h"

#include <fstream>

#include "comet/core/compression.h"
#include "comet/core/file_system/file_system.h"
#include "comet/core/generator.h"
#include "comet/core/logger.h"
#include "comet/core/memory/memory.h"

namespace comet {
namespace resource {
Resource* ResourceCache::Set(std::unique_ptr<Resource> resource) {
  const auto it{cache_.emplace(resource->id, std::move(resource))};
  COMET_ASSERT(it.second, "Unable to save resource to cache!");
  return it.first->second.get();
}

Resource* ResourceCache::Get(ResourceId resource_id) {
  if (cache_.find(resource_id) == cache_.cend()) {
    return nullptr;
  }

  return cache_.at(resource_id).get();
}

void ResourceCache::Destroy(ResourceId resource_id) {
  COMET_ASSERT(cache_.find(resource_id) != cache_.cend(),
               "Tried to destroy resource with ID ",
               COMET_STRING_ID_LABEL(resource_id), ", but the former is null!");
  cache_.erase(resource_id);
}

ResourceId GenerateResourceIdFromPath(CTStringView resource_path) {
  return COMET_STRING_ID(resource_path);
}

void PackBytes(const u8* bytes, usize bytes_size,
               CompressionMode compression_mode, std::vector<u8>* packed_bytes,
               usize* packed_bytes_size) {
  switch (compression_mode) {
    case CompressionMode::Lz4: {
      CompressLz4(bytes, bytes_size, *packed_bytes);
      *packed_bytes_size = packed_bytes->size();
      break;
    }
    case CompressionMode::None: {
      memory::CopyMemory(packed_bytes, bytes, bytes_size);
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
               std::vector<u8>* packed_bytes, usize* packed_bytes_size) {
  PackBytes(bytes.data(), bytes.size(), compression_mode, packed_bytes,
            packed_bytes_size);
}

void PackResourceData(const std::vector<u8>& data, ResourceFile& file) {
  file.data_size = data.size();
  PackBytes(data, file.compression_mode, &file.data, &file.packed_data_size);
}

std::vector<u8> UnpackBytes(CompressionMode compression_mode,
                            const u8* packed_bytes, usize packed_bytes_size,
                            usize decompressed_size) {
  std::vector<u8> data;

  switch (compression_mode) {
    case CompressionMode::Lz4: {
      DecompressLz4(packed_bytes, packed_bytes_size, decompressed_size, data);
      break;
    }
    case CompressionMode::None: {
      data.resize(decompressed_size);
      memory::CopyMemory(data.data(), packed_bytes, decompressed_size);
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
                            usize decompressed_size) {
  return UnpackBytes(compression_mode, packed_bytes.data(), packed_bytes.size(),
                     decompressed_size);
}

std::vector<u8> UnpackResourceData(const ResourceFile& file) {
  return UnpackBytes(file.compression_mode, file.data, file.data_size);
}

bool SaveResourceFile(CTStringView path, const ResourceFile& file) {
  std::ofstream out_file;

  if (!OpenFileToWriteTo(path, out_file, false, true)) {
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

  CloseFile(out_file);
  return true;
}

bool LoadResourceFile(CTStringView path, ResourceFile& file) {
  std::ifstream in_file;

  if (!OpenFileToReadFrom(path, in_file, false, true)) {
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

const Resource* ResourceHandler::Load(CTStringView root_resource_path,
                                      CTStringView resource_path) {
  return Load(root_resource_path, GenerateResourceIdFromPath(resource_path));
}

const Resource* ResourceHandler::Load(CTStringView root_resource_path,
                                      ResourceId resource_id) {
  if (resource_id == kDefaultResourceId) {
    return GetDefaultResource();
  }

  auto* resource{GetInternal(resource_id)};

  if (resource != nullptr) {
    ++resource->ref_count;
    return resource;
  }

  tchar resource_id_path[10];
  usize resource_id_path_len;
  ConvertToStr(resource_id, resource_id_path, 10, &resource_id_path_len);

  TString resource_abs_path{};
  // Adding 1 for the hypothetical "/" between those two (if needed).
  resource_abs_path.Reserve(root_resource_path.GetLength() + 1 +
                            resource_id_path_len);
  resource_abs_path = root_resource_path;
  resource_abs_path /= resource_id_path;  // No allocation.

  ResourceFile file{};
  const auto is_load{LoadResourceFile(resource_abs_path, file)};

  if (!is_load) {
    COMET_LOG_RESOURCE_ERROR("Unable to get resource with ID: ",
                             COMET_STRING_ID_LABEL(resource_id), ".");
    return GetDefaultResource();
  }

  resource = cache_.Set(Unpack(file));
  resource->ref_count = 1;
  return resource;
}

void ResourceHandler::Unload(CTStringView resource_path) {
  Unload(GenerateResourceIdFromPath(resource_path));
}

void ResourceHandler::Unload(ResourceId resource_id) {
  COMET_ASSERT(resource_id != kDefaultResourceId,
               "Cannot unload default resource!");
  auto* resource{cache_.Get(resource_id)};
  COMET_ASSERT(resource != nullptr, "Tried to unload resource with ID ",
               COMET_STRING_ID_LABEL(resource_id), ", but the former is null!");
  COMET_ASSERT(resource->ref_count > 0, "Resource with ID ",
               COMET_STRING_ID_LABEL(resource_id),
               ", was asked to be unloaded, but reference count is already 0! "
               "What happened??");
  --resource->ref_count;

  if (resource->ref_count < 1) {
    Destroy(resource_id);
  }
}

const Resource* ResourceHandler::Get(ResourceId resource_id) {
  return GetInternal(resource_id);
}

void ResourceHandler::Destroy(ResourceId resource_id) {
  cache_.Destroy(resource_id);
}

const Resource* ResourceHandler::GetDefaultResource() { return nullptr; }

ResourceFile ResourceHandler::GetResourceFile(
    const Resource& resource, CompressionMode compression_mode) const {
  return Pack(resource, compression_mode);
}

Resource* ResourceHandler::GetInternal(ResourceId resource_id) {
  return cache_.Get(resource_id);
}
}  // namespace resource
}  // namespace comet
