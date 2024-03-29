// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "asset_exporter.h"

#include "comet/core/c_string.h"
#include "editor/asset/asset_utils.h"

namespace comet {
namespace editor {
namespace asset {
const TString& AssetExporter::GetRootResourcePath() const {
  return root_resource_path_;
}

const TString& AssetExporter::GetRootAssetPath() const {
  return root_asset_path_;
}

bool AssetExporter::Process(CTStringView asset_abs_path) {
  COMET_LOG_GLOBAL_INFO("Processing asset at path: ", asset_abs_path, ".");

  AssetDescr descr{};
  descr.asset_abs_path = asset_abs_path;

  Clean(descr.asset_abs_path.GetTStr(), descr.asset_abs_path.GetLength());
  descr.asset_path = GetRelativePath(descr.asset_abs_path, root_asset_path_);
  descr.metadata_path = GenerateAssetMetadataFilePath(descr.asset_abs_path);
  descr.metadata = SetAndGetMetadata(descr.metadata_path);
  const schar* compression_mode_label{nullptr};

  switch (compression_mode_) {
    case resource::CompressionMode::Lz4:
      compression_mode_label = kCometResourceCompressionModeLz4.data();
      break;
    case resource::CompressionMode::None:
      compression_mode_label = kCometResourceCompressionModeNone.data();
      break;
    default:
      COMET_LOG_GLOBAL_ERROR(
          "Unknown compression mode: ",
          static_cast<std::underlying_type_t<resource::CompressionMode>>(
              compression_mode_),
          " for asset at path ", descr.asset_path, ". Ignoring compression.");
      compression_mode_label = kCometResourceCompressionModeNone.data();
      break;
  }

  descr.metadata[kCometEditorAssetMetadataKeyCompressionMode] =
      compression_mode_label;

  auto resource_files{GetResourceFiles(descr)};

  if (resource_files.size() == 0) {
    COMET_LOG_GLOBAL_ERROR("Could not process asset at ", descr.asset_abs_path);
    return false;
  }

  for (const auto& resource_file : resource_files) {
    if (!resource::SaveResourceFile(
            GenerateResourcePath(root_resource_path_,
                                 resource_file.resource_id),
            resource_file)) {
      COMET_LOG_GLOBAL_ERROR("Unable to save resource file: ",
                             resource_file.resource_id);
    }
  }

  SaveMetadata(descr.metadata_path, descr.metadata);
  return true;
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
