// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_RESOURCE_H_
#define COMET_COMET_RESOURCE_RESOURCE_H_

#include "comet/core/compression.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/string_id.h"
#include "comet/core/type/tstring.h"
#include "comet/core/type_trait.h"
#include "comet/resource/label/common_label.h"
#include "comet/resource/type/common.h"

namespace comet {
namespace resource {
struct ResourceFile {
  RawResourceId resource_id{kInvalidRawResourceId};
  ResourceTypeId resource_type_id{kInvalidResourceTypeId};
  CompressionMode compression_mode{CompressionMode::None};
  usize descr_size{0};
  usize data_size{0};
  usize packed_descr_size{0};
  usize packed_data_size{0};
  Array<u8> descr{};
  Array<u8> data{};
};

struct Resource {
  ResourceLifeSpan life_span{ResourceLifeSpan::Unknown};
  RawResourceId id{kInvalidRawResourceId};
  ResourceTypeId type_id{kInvalidResourceTypeId};

  virtual ~Resource() = default;
};

struct InternalResource {
  RawResourceId resource_id{kInvalidRawResourceId};
  RawResourceId internal_id{kInvalidRawResourceId};
};

using ResourcePtr = memory::CustomUniquePtr<Resource>;

template <typename ResourceType>
using ResourceIdOf = typename ResourceType::Id;

template <typename ResourceType>
ResourceIdOf<ResourceType> GenerateResourceIdFromPath(
    CTStringView resource_path) {
  return ResourceIdOf<ResourceType>{HashCombine(COMET_STRING_ID(resource_path),
                                                ResourceType::kResourceTypeId)};
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
  constexpr auto kDecompressedSize{sizeof(T)};

  switch (compression_mode) {
    case CompressionMode::Lz4: {
      DecompressLz4(packed_bytes, packed_bytes_size, kDecompressedSize,
                    reinterpret_cast<u8*>(&data));
      break;
    }
    case CompressionMode::None: {
      memory::CopyMemory(&data, packed_bytes, kDecompressedSize);
      break;
    }
    default: {
      COMET_ASSERT(false, "resource::UnpackBytes", "unknown compression mode",
                   "compression_mode",
                   GetCompressionModeLabel(compression_mode),
                   "compression_mode_value", ToUnderlying(compression_mode));
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
