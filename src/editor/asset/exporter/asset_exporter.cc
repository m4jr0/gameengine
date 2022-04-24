// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "asset_exporter.h"

#include "comet/utils/file_system.h"
#include "editor/asset/asset_utils.h"

namespace comet {
namespace editor {
namespace asset {
void AssetExporter::Initialize() {}

void AssetExporter::Destroy() {}

const std::string& AssetExporter::GetRootResourcePath() {
  return root_resource_path_;
}

const std::string& AssetExporter::GetRootAssetPath() {
  return root_asset_path_;
}

bool AssetExporter::Process(const std::string& asset_abs_path) {
  COMET_LOG_GLOBAL_INFO("Processing asset at path: ", asset_abs_path, ".");

  AssetDescr descr{};
  descr.asset_abs_path = asset_abs_path;
  descr.asset_path = utils::filesystem::GetRelativePath(descr.asset_abs_path,
                                                        root_asset_path_);
  descr.metadata_path = GetAssetMetadataFilePath(descr.asset_abs_path);
  descr.resource_id = resource::GenerateResourceId(descr.asset_path);
  auto is_metadata_error{false};
  descr.metadata = SetAndGetMetadata(descr.metadata_path);

  if (!AttachResourceToAssetDescr(descr)) {
    COMET_LOG_GLOBAL_ERROR("Could not process asset at ", descr.asset_abs_path);
    return false;
  }

  const auto compression_mode{descr.resource.compression_mode};
  const char* compression_mode_label{nullptr};

  switch (compression_mode) {
    case resource::CompressionMode::Lz4:
      compression_mode_label = kCometResourceCompressionModeLz4;
      break;
    case resource::CompressionMode::None:
      compression_mode_label = kCometResourceCompressionModeNone;
      break;
    default:
      COMET_LOG_GLOBAL_ERROR(
          "Unknown compression mode: ",
          static_cast<std::underlying_type_t<resource::CompressionMode>>(
              compression_mode),
          " for asset at path ", descr.asset_path, ". Ignoring compression.");
      compression_mode_label = kCometResourceCompressionModeNone;
      break;
  }

  descr.metadata[kCometEditorAssetMetadataKeyCompressionMode] =
      compression_mode_label;

  SaveMetadata(descr.metadata_path, descr.metadata);

  if (!resource::SaveResourceFile(
          utils::filesystem::Append(root_resource_path_,
                                    std::to_string(descr.resource_id)),
          descr.resource)) {
    COMET_LOG_GLOBAL_ERROR("Unable to save resource file: ", descr.resource_id);
    return false;
  }

  return true;
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
