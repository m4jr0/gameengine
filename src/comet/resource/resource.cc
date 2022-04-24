// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "resource.h"

#include <fstream>

#include "comet/utils/file_system.h"

namespace comet {
namespace resource {
void PackResourceData(const std::vector<char>& data, ResourceFile& file) {
  file.data_size = sizeof(char) * data.size();

  switch (file.compression_mode) {
    case CompressionMode::Lz4: {
      utils::compression::CompressLz4(data, file.data_size, file.data);
      file.packed_data_size = sizeof(file.data[0]) * file.data.size();
      break;
    }
    case CompressionMode::None: {
      file.data = {data.data(), data.data() + data.size()};
      file.packed_data_size = file.data_size;
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

std::vector<char> UnpackResourceData(const ResourceFile& file) {
  std::vector<char> data;

  switch (file.compression_mode) {
    case CompressionMode::Lz4: {
      utils::compression::DecompressLz4(file.data, file.data_size, data);
      break;
    }
    case CompressionMode::None: {
      data.resize(file.data_size);
      std::memcpy(reinterpret_cast<void*>(data.data()),
                  reinterpret_cast<const void*>(file.data.data()),
                  file.data_size);
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

  return data;
}

bool SaveResourceFile(const std::string& path, const ResourceFile& file) {
  std::ofstream out_file;

  if (!utils::filesystem::OpenBinaryFileToWriteTo(path, out_file, false)) {
    COMET_LOG_RESOURCE_ERROR("Unable to write resource file: ", path);
    return false;
  }

  out_file.write(reinterpret_cast<const char*>(&file.resource_type_id),
                 static_cast<std::streamsize>(sizeof(file.resource_type_id)));

  out_file.write(reinterpret_cast<const char*>(&file.compression_mode),
                 static_cast<std::streamsize>(sizeof(file.compression_mode)));

  out_file.write(reinterpret_cast<const char*>(&file.packed_descr_size),
                 static_cast<std::streamsize>(sizeof(file.packed_descr_size)));

  out_file.write(reinterpret_cast<const char*>(&file.packed_data_size),
                 static_cast<std::streamsize>(sizeof(file.packed_data_size)));

  out_file.write(reinterpret_cast<const char*>(&file.descr_size),
                 static_cast<std::streamsize>(sizeof(file.descr_size)));

  out_file.write(reinterpret_cast<const char*>(&file.data_size),
                 static_cast<std::streamsize>(sizeof(file.data_size)));

  out_file.write(reinterpret_cast<const char*>(file.descr.data()),
                 file.packed_descr_size);

  out_file.write(reinterpret_cast<const char*>(file.data.data()),
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
  in_file.read(reinterpret_cast<char*>(&file.resource_type_id),
               sizeof(ResourceId));

  in_file.read(reinterpret_cast<char*>(&file.compression_mode),
               sizeof(file.compression_mode));

  in_file.read(reinterpret_cast<char*>(&file.packed_descr_size),
               sizeof(file.packed_descr_size));

  in_file.read(reinterpret_cast<char*>(&file.packed_data_size),
               sizeof(file.packed_data_size));

  in_file.read(reinterpret_cast<char*>(&file.descr_size),
               sizeof(file.descr_size));

  in_file.read(reinterpret_cast<char*>(&file.data_size),
               sizeof(file.data_size));

  file.descr.resize(file.packed_descr_size);
  in_file.read(file.descr.data(), file.packed_descr_size);

  file.data.resize(file.packed_data_size);
  in_file.read(file.data.data(), file.packed_data_size);

  return true;
}

ResourceFile ResourceHandler::GetResourceFile(
    const Resource& resource, CompressionMode compression_mode) const {
  return Pack(resource, compression_mode);
}
}  // namespace resource
}  // namespace comet
