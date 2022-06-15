// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_ASSET_UTILS_H_
#define COMET_EDITOR_ASSET_ASSET_UTILS_H_

#include "comet_precompile.h"

#include "nlohmann/json.hpp"

#include "comet/utils/date.h"
#include "comet/utils/file_system.h"
#include "editor/asset/asset.h"

namespace comet {
namespace editor {
namespace asset {
template <typename AssetFilePath>
std::string GetAssetMetadataFilePath(AssetFilePath&& asset_file_path) {
  return std::forward<AssetFilePath>(asset_file_path) + "." +
         kCometEditorAssetMetadataFileExtension;
}

template <typename MetadataFilePath>
void SaveMetadata(MetadataFilePath&& meta_file_path,
                  const nlohmann::json& metadata) {
  utils::filesystem::WriteStrToFile(
      std::forward<MetadataFilePath>(meta_file_path),
      metadata.dump(kCometEditorAssetMetadataIndent));
}

template <typename MetadataFilePath>
nlohmann::json GetMetadata(MetadataFilePath&& meta_file_path) {
  if (!utils::filesystem::IsFile(meta_file_path)) {
    // We have to use () here, with {} the wrong type (array) is set.
    return nlohmann::json(nlohmann::json::value_t::object);
  }

  std::string metadata_raw;

  try {
    utils::filesystem::ReadStrFromFile(
        std::forward<MetadataFilePath>(meta_file_path), metadata_raw);

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

template <typename MetadataFilePath>
nlohmann::json SetAndGetMetadata(MetadataFilePath&& meta_file_path) {
  // We must use assignment here to prevent a bug with GCC where the generated
  // type is an array (which is wrong).
  auto metadata = GetMetadata(std::forward<MetadataFilePath>(meta_file_path));

  const auto comet_version{version::GetFormattedVersion()};
  const auto update_time{utils::date::GetNow()};
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

bool IsMetadataFile(const std::string& file_path);
bool IsMetadataFile(const char* file_path);
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_ASSET_UTILS_H_
