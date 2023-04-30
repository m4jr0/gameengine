// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "asset_utils.h"

#include "comet/core/date.h"
#include "comet/core/file_system.h"
#include "editor/asset/asset.h"

namespace comet {
namespace editor {
namespace asset {
std::string GetAssetMetadataFilePath(std::string_view asset_file_path) {
  if (asset_file_path.empty()) {
    return std::string{};
  }

  auto asset_file_path_size{asset_file_path.size()};
  auto extension_size{kCometEditorAssetMetadataFileExtension.size()};

  std::string metadata_file_path{};
  // + 1 to add the dot.
  metadata_file_path.resize(asset_file_path_size + extension_size + 1);

  for (uindex i{0}; i < asset_file_path_size; ++i) {
    metadata_file_path[i] = asset_file_path[i];
  }

  metadata_file_path[asset_file_path_size] = '.';

  for (uindex i{0}; i < extension_size; ++i) {
    metadata_file_path[i + asset_file_path_size + 1] =
        kCometEditorAssetMetadataFileExtension[i];
  }

  return metadata_file_path;
}

void SaveMetadata(const schar* metadata_file_path,
                  const nlohmann::json& metadata) {
  WriteStrToFile(metadata_file_path,
                 metadata.dump(kCometEditorAssetMetadataIndent));
}

void SaveMetadata(const std::string& metadata_file_path,
                  const nlohmann::json& metadata) {
  return SaveMetadata(metadata_file_path.c_str(), metadata);
}

nlohmann::json GetMetadata(const schar* metadata_file_path) {
  if (!IsFile(metadata_file_path)) {
    // We have to use () here, with {} the wrong type (array) is set.
    return nlohmann::json(nlohmann::json::value_t::object);
  }

  std::string metadata_raw;

  try {
    ReadStrFromFile(metadata_file_path, metadata_raw);

    if (metadata_raw.size() == 0) {
      // Same here.
      return nlohmann::json(nlohmann::json::value_t::object);
    }

    return nlohmann::json::parse(metadata_raw);
  } catch (const nlohmann::json::exception& error) {
    COMET_LOG_GLOBAL_ERROR("An error occurred while processing JSON file: ",
                           error.what());
  }

  // Same here.
  return nlohmann::json(nlohmann::json::value_t::object);
}

nlohmann::json GetMetadata(const std::string& metadata_file_path) {
  return GetMetadata(metadata_file_path.c_str());
}

nlohmann::json SetAndGetMetadata(const schar* metadata_file_path) {
  // We must use assignment here to prevent a bug with GCC where the generated
  // type is an array (which is wrong).
  auto metadata = GetMetadata(metadata_file_path);
  const auto comet_version{version::GetFormattedVersion()};
  const auto update_time{GetNow()};
  f64 creation_time{0};
  uindex file_version{0};

  try {
    creation_time =
        metadata.value(kCometEditorAssetMetadataKeyVersion, update_time);
    file_version =
        metadata.value(kCometEditorAssetMetadataKeyVersion, file_version);
  } catch (const nlohmann::json::exception& error) {
    COMET_LOG_GLOBAL_ERROR("An error occurred while processing JSON file: ",
                           error.what(), ". Resetting it.");
    metadata = nlohmann::json{};
  }

  metadata[kCometEditorAssetMetadataKeyCreationTime] = creation_time;
  metadata[kCometEditorAssetMetadataKeyUpdateTime] = update_time;
  metadata[kCometEditorAssetMetadataKeyVersion] = file_version + 1;
  metadata[kCometEditorAssetCometVersion] = comet_version;

  return metadata;
}

nlohmann::json SetAndGetMetadata(const std::string& metadata_file_path) {
  return SetAndGetMetadata(metadata_file_path.c_str());
}

bool IsMetadataFile(const std::string& file_path) {
  return IsMetadataFile(file_path.c_str());
}

bool IsMetadataFile(const schar* file_path) {
  return GetExtension(file_path) == kCometEditorAssetMetadataFileExtension;
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
