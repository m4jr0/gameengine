// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_RESOURCE_H_
#define COMET_COMET_RESOURCE_RESOURCE_H_

// External. ///////////////////////////////////////////////////////////////////
#include <type_traits>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/compression.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/string_id.h"
#include "comet/core/type/tstring.h"

namespace comet {
namespace resource {
using ResourceId = stringid::StringId;
constexpr auto kInvalidResourceId{static_cast<ResourceId>(-1)};
using ResourceTypeId = stringid::StringId;
constexpr auto kDefaultResourceId{0};
constexpr auto kInvalidResourceTypeId{static_cast<ResourceTypeId>(-1)};

enum class CompressionMode : u8 { None = 0, Lz4 };

enum class ResourceLifeSpan : u8 { Unknown = 0, Manual, Scene, Global };

struct ResourceFile {
  ResourceId resource_id{kInvalidResourceId};
  ResourceId resource_type_id{kInvalidResourceTypeId};
  CompressionMode compression_mode{CompressionMode::None};
  usize descr_size{0};
  usize data_size{0};
  usize packed_descr_size{0};
  usize packed_data_size{0};
  Array<u8> descr{};
  Array<u8> data{};
};

using RefCount = u32;
constexpr auto kInvalidRefCount{static_cast<RefCount>(-1)};

struct Resource {
  ResourceLifeSpan life_span{ResourceLifeSpan::Unknown};
  ResourceId id{kInvalidResourceId};
  ResourceId type_id{kInvalidResourceTypeId};
  RefCount ref_count{kInvalidRefCount};

  virtual ~Resource() = default;
};

struct InternalResource {
  ResourceId resource_id{kInvalidResourceId};
  ResourceId internal_id{kInvalidResourceId};
};

using ResourcePtr = memory::CustomUniquePtr<Resource>;

template <typename ResourceType>
ResourceId GenerateResourceIdFromPath(CTStringView resource_path) {
  return HashCombine(COMET_STRING_ID(resource_path),
                     ResourceType::kResourceTypeId);
}

void PackBytes(const u8* bytes, usize bytes_size,
               CompressionMode compression_mode, Array<u8>* packed_bytes,
               usize* packed_bytes_size);
void PackBytes(const Array<u8>& bytes, CompressionMode compression_mode,
               Array<u8>* packed_bytes, usize* packed_bytes_size);
void PackResourceData(const Array<u8>& data, ResourceFile& file);

template <typename ResourceDescrType>
void PackPodResourceDescr(const ResourceDescrType& descr, ResourceFile& file) {
  file.descr_size = sizeof(descr);
  PackBytes(reinterpret_cast<const u8*>(&descr), file.descr_size,
            file.compression_mode, &file.descr, &file.packed_descr_size);
}

template <typename T>
void UnpackBytes(CompressionMode compression_mode, const u8* packed_bytes,
                 usize packed_bytes_size, T& data) {
  auto decompressed_size{sizeof(T)};

  switch (compression_mode) {
    case CompressionMode::Lz4: {
      DecompressLz4(packed_bytes, packed_bytes_size, decompressed_size,
                    reinterpret_cast<u8*>(&data));
      break;
    }
    case CompressionMode::None: {
      memory::CopyMemory(&data, packed_bytes, decompressed_size);
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

void UnpackBytes(CompressionMode compression_mode, const u8* packed_bytes,
                 usize packed_bytes_size, usize decompressed_size,
                 Array<u8>& data);
void UnpackBytes(CompressionMode compression_mode,
                 const Array<u8>& packed_bytes, usize decompressed_size,
                 Array<u8>& data);
void UnpackResourceData(const ResourceFile& file, Array<u8>& data,
                        usize max_data_size = kInvalidSize);

template <typename ResourceDescrType>
void UnpackPodResourceDescr(const ResourceFile& file,
                            ResourceDescrType& descr) {
  UnpackBytes(file.compression_mode, file.descr.GetData(),
              file.packed_descr_size, descr);
}

bool SaveResourceFile(CTStringView path, const ResourceFile& file);
bool LoadResourceFile(CTStringView path, ResourceFile& file);
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_RESOURCE_H_
