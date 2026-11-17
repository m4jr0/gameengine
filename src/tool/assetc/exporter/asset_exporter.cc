// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "assetc_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "asset_exporter.h"
////////////////////////////////////////////////////////////////////////////////

#include "asset_utils.h"
#include "comet/core/job/job_utils.h"
#include "comet/core/job/scheduler.h"
#include "comet/core/logger/logging.h"
#include "comet/core/string/c_string.h"
#include "comet/core/type_trait.h"
#include "comet/data/resource/label/common_label.h"

#ifdef COMET_FIBER_DEBUG_LABEL
#include "comet/core/fiber/fiber.h"
#endif  // COMET_FIBER_DEBUG_LABEL

namespace comet {
namespace tool {
namespace assetc {
const TString& AssetExporter::GetRootResourcePath() const {
  return root_resource_path_;
}

const TString& AssetExporter::GetRootAssetPath() const {
  return root_asset_path_;
}

void AssetExporter::Process(const AssetExportDescr& export_descr) {
  COMET_ASSERT(export_descr.allocator != nullptr, "AssetExporter::Process",
               "allocator is null");
  ResourceFilesContext context{};
  context.allocator = export_descr.allocator;
  context.files = ResourceFiles{context.allocator};
  auto& descr{context.asset_descr};
  descr.asset_abs_path = export_descr.asset_abs_path;
  Clean(descr.asset_abs_path.GetTStr(), descr.asset_abs_path.GetLength());
  descr.asset_path = GetRelativePath(descr.asset_abs_path, root_asset_path_);
  descr.metadata_path = GenerateAssetMetadataFilePath(descr.asset_abs_path);
  descr.metadata = SetAndGetMetadata(descr.metadata_path);
  descr.metadata[kCometEditorAssetMetadataKeyCompressionMode] =
      compression_mode_ == resource::CompressionMode::Lz4
          ? kCometResourceCompressionModeLz4.data()
          : kCometResourceCompressionModeNone.data();

  // PopulateFiles may schedule jobs internally; it must finish before the
  // resource files and metadata are written or their storage is released.
  PopulateFiles(context);
  if (context.files.IsEmpty()) {
    COMET_LOG_WARNING(LoggerType::External, "AssetExporter::Process",
                      "asset produced no resources", "asset_path",
                      descr.asset_abs_path);
    return;
  }

  auto resource_files = nlohmann::json::array();
  constexpr auto kBufferSize{GetCharCount<resource::RawResourceId>() + 1};
  schar buffer[kBufferSize]{'\\0'};
  usize out_len{0};
  for (const auto& resource_file : context.files) {
    ConvertToStr(resource_file.resource_id, buffer, kBufferSize, &out_len);
    resource_files.push_back(std::string{buffer, out_len});
    if (!resource::SaveResourceFile(
            GenerateResourcePath(root_resource_path_, resource_file.resource_id),
            resource_file)) {
      COMET_LOG_ERROR(LoggerType::External, "AssetExporter::Process",
                      "resource file save failed", "resource_id",
                      resource_file.resource_id);
      return;
    }
  }
  descr.metadata[kCometEditorAssetMetadataKeyResourceFiles] = resource_files;
  SaveMetadata(descr.metadata_path, descr.metadata);
}

}  // namespace assetc
}  // namespace tool
}  // namespace comet
