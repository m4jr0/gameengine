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
TString GenerateAssetMetadataFilePath(CTStringView asset_file_path) {
  if (IsEmpty(asset_file_path)) {
    return {};
  }

  auto extension_size{kCometEditorAssetMetadataFileExtension.GetLength()};

  // + 1 to add the dot.
  auto total_len{asset_file_path.GetLength() +
                 kCometEditorAssetMetadataFileExtension.GetLength() + 1};
  TString path{};
  path.Resize(total_len);
  COMET_DISALLOW_STR_ALLOC(path);

  for (uindex i{0}; i < asset_file_path.GetLength(); ++i) {
    path[i] = asset_file_path[i];
  }

  path[asset_file_path.GetLength()] = COMET_TCHAR('.');

  for (uindex i{0}; i < kCometEditorAssetMetadataFileExtension.GetLength();
       ++i) {
    path[i + asset_file_path.GetLength() + 1] =
        kCometEditorAssetMetadataFileExtension[i];
  }

  return path;
}

void SaveMetadata(CTStringView metadata_file_path,
                  const nlohmann::json& metadata) {
  auto str_dump{metadata.dump(kCometEditorAssetMetadataIndent)};
  WriteStrToFile(metadata_file_path, str_dump.c_str());
}

nlohmann::json GetMetadata(CTStringView metadata_file_path) {
  if (!IsFile(metadata_file_path)) {
    // We have to use () here, with {} the wrong type (array) is set.
    return nlohmann::json(nlohmann::json::value_t::object);
  }

  constexpr auto kMaxMetadataRaw{4096};
  schar metadata_raw[kMaxMetadataRaw];

  try {
    uindex metadata_raw_len;
    ReadStrFromFile(metadata_file_path, metadata_raw, kMaxMetadataRaw,
                    &metadata_raw_len);

    if (metadata_raw_len == 0) {
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

nlohmann::json SetAndGetMetadata(CTStringView metadata_file_path) {
  // We must use assignment here to prevent a bug with GCC where the generated
  // type is an array (which is wrong).
  auto metadata = GetMetadata(metadata_file_path);
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
  metadata[kCometEditorAssetCometVersion] = version::GetVersionStr();

  return metadata;
}

bool IsMetadataFile(CTStringView file_path) {
  return GetExtension(file_path) == kCometEditorAssetMetadataFileExtension;
}

TString GenerateResourcePath(CTStringView folder_path,
                             resource::ResourceId resource_id) {
  // The ID will be 10 characters max.
  tchar resource_id_path[11];
  uindex resource_id_path_len;
  ConvertToStr(resource_id, resource_id_path, 10, &resource_id_path_len);
  resource_id_path[10] = COMET_TCHAR('\0');
  return folder_path / resource_id_path;
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
