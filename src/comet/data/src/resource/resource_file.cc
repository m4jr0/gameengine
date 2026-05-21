// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/data/resource_file.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <fstream>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_string.h"
#include "comet/core/compression.h"
#include "comet/core/file_system/file_system.h"
#include "comet/core/logger/logging.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type_trait.h"
#include "comet/math/math_scalar.h"
#include "comet/profiler/profiler.h"

namespace comet {
namespace resource {
void PackBytes(const u8* bytes, usize bytes_size,
               CompressionMode compression_mode, Array<u8>* packed_bytes,
               usize* packed_bytes_size) {
  COMET_PROFILE("resource::PackBytes");
  COMET_ASSERT(bytes_size == 0 || bytes != nullptr, "resource::PackBytes",
               "bytes are null for non-zero size", "bytes_size", bytes_size);
  COMET_ASSERT(packed_bytes != nullptr, "resource::PackBytes",
               "packed bytes output is null");
  COMET_ASSERT(packed_bytes_size != nullptr, "resource::PackBytes",
               "packed bytes size output is null");

  switch (compression_mode) {
    case CompressionMode::Lz4: {
      CompressLz4(bytes, bytes_size, *packed_bytes);
      *packed_bytes_size = packed_bytes->GetSize();
      break;
    }
    case CompressionMode::None: {
      packed_bytes->Resize(bytes_size);
      memory::CopyMemory(packed_bytes->GetData(), bytes, bytes_size);
      *packed_bytes_size = bytes_size;
      break;
    }
    default: {
      COMET_ASSERT(false, "resource::PackBytes", "unknown compression mode",
                   "compression_mode",
                   GetCompressionModeLabel(compression_mode),
                   "compression_mode_value", ToUnderlying(compression_mode));
      return;
    }
  }
}

void PackBytes(const Array<u8>& bytes, CompressionMode compression_mode,
               Array<u8>* packed_bytes, usize* packed_bytes_size) {
  PackBytes(bytes.GetData(), bytes.GetSize(), compression_mode, packed_bytes,
            packed_bytes_size);
}

void PackResourceData(const Array<u8>& data, ResourceFile& file) {
  COMET_PROFILE("resource::PackResourceData");
  file.data_size = data.GetSize();
  PackBytes(data, file.compression_mode, &file.data, &file.packed_data_size);
}

void UnpackBytes(CompressionMode compression_mode, const u8* packed_bytes,
                 usize packed_bytes_size, usize decompressed_size,
                 Array<u8>& data) {
  COMET_PROFILE("resource::UnpackBytes");
  COMET_ASSERT(packed_bytes_size == 0 || packed_bytes != nullptr,
               "resource::UnpackBytes",
               "packed bytes are null for non-zero size", "packed_bytes_size",
               packed_bytes_size);
  COMET_ASSERT(decompressed_size == 0 || packed_bytes != nullptr,
               "resource::UnpackBytes",
               "packed bytes are null for non-zero decompressed size",
               "decompressed_size", decompressed_size);

  switch (compression_mode) {
    case CompressionMode::Lz4: {
      DecompressLz4(packed_bytes, packed_bytes_size, decompressed_size, data);
      break;
    }
    case CompressionMode::None: {
      data.Resize(decompressed_size);
      memory::CopyMemory(data.GetData(), packed_bytes, decompressed_size);
      break;
    }
    default: {
      COMET_ASSERT(false, "resource::UnpackBytes", "unknown compression mode",
                   "compression_mode",
                   GetCompressionModeLabel(compression_mode),
                   "compression_mode_value", ToUnderlying(compression_mode));
      return;
    }
  }
}

void UnpackBytes(CompressionMode compression_mode,
                 const Array<u8>& packed_bytes, usize decompressed_size,
                 Array<u8>& data) {
  UnpackBytes(compression_mode, packed_bytes.GetData(), packed_bytes.GetSize(),
              decompressed_size, data);
}

void UnpackResourceData(const ResourceFile& file, Array<u8>& data,
                        usize max_data_size) {
  COMET_PROFILE("resource::UnpackResourceData");
  COMET_ASSERT(max_data_size == kInvalidSize || max_data_size <= file.data_size,
               "resource::UnpackResourceData",
               "max data size exceeds file data size", "max_data_size",
               max_data_size, "file_data_size", file.data_size);
  COMET_ASSERT(file.packed_data_size == 0 || file.data.GetData() != nullptr,
               "resource::UnpackResourceData",
               "packed file data is null for non-zero size", "packed_data_size",
               file.packed_data_size);

  const auto data_size{max_data_size != kInvalidSize
                           ? math::Min(max_data_size, file.data_size)
                           : file.data_size};

  UnpackBytes(file.compression_mode, file.data, data_size, data);
}

bool SaveResourceFile(CTStringView path, const ResourceFile& file) {
  COMET_PROFILE("resource::SaveResourceFile");
  COMET_ASSERT(!path.IsEmpty(), "resource::SaveResourceFile", "path is empty");
  COMET_ASSERT(file.packed_descr_size == 0 || file.descr.GetData() != nullptr,
               "resource::SaveResourceFile",
               "descriptor payload is null for non-zero size",
               "packed_descr_size", file.packed_descr_size);
  COMET_ASSERT(file.packed_data_size == 0 || file.data.GetData() != nullptr,
               "resource::SaveResourceFile",
               "data payload is null for non-zero size", "packed_data_size",
               file.packed_data_size);

  std::ofstream out_file;

  if (!OpenFileToWriteTo(path, out_file, false, true)) {
    COMET_LOG_ERROR(LoggerType::Resource, "resource::SaveResourceFile",
                    "unable to open file for writing", "path", path);
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

  out_file.write(reinterpret_cast<const schar*>(file.descr.GetData()),
                 file.packed_descr_size);

  out_file.write(reinterpret_cast<const schar*>(file.data.GetData()),
                 file.packed_data_size);

  COMET_ASSERT(out_file.good(), "resource::SaveResourceFile",
               "failed to write resource file", "path", path);

  CloseFile(out_file);
  return true;
}

bool ReadResourceFile(CTStringView path, ResourceFile& file) {
  COMET_ASSERT(!path.IsEmpty(), "resource::ReadResourceFile", "path is empty");

  std::ifstream in_file;

  if (!OpenFileToReadFrom(path, in_file, false, true)) {
    COMET_LOG_ERROR(LoggerType::Resource, "resource::ReadResourceFile",
                    "unable to open resource file", "path", path);
    return false;
  }

  in_file.seekg(0);

  in_file.read(reinterpret_cast<schar*>(&file.resource_type_id),
               sizeof(ResourceTypeId));
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

  if (!in_file.good()) {
    COMET_LOG_ERROR(LoggerType::Resource, "resource::ReadResourceFile",
                    "failed to read resource file header", "path", path);
    CloseFile(in_file);
    return false;
  }

  file.descr.Resize(file.packed_descr_size);
  file.data.Resize(file.packed_data_size);

  in_file.read(reinterpret_cast<schar*>(file.descr.GetData()),
               file.packed_descr_size);
  in_file.read(reinterpret_cast<schar*>(file.data.GetData()),
               file.packed_data_size);

  if (!in_file.good()) {
    COMET_LOG_ERROR(LoggerType::Resource ", " resource::ReadResourceFile
                    ",
                    "failed to read resource file payload",
                    "path", path);
    CloseFile(in_file);
    return false;
  }

  CloseFile(in_file);
  return true;
}
}  // namespace resource
}  // namespace comet
