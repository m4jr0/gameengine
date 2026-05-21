// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "asset_exporter.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_string.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/logger/logging.h"
#include "comet/core/type_trait.h"
#include "comet/resource/label/common_label.h"
#include "editor/asset/asset_utils.h"

#ifdef COMET_FIBER_DEBUG_LABEL
#include "comet/core/concurrency/fiber/fiber.h"
#endif  // COMET_FIBER_DEBUG_LABEL

namespace comet {
namespace editor {
namespace asset {
const TString& AssetExporter::GetRootResourcePath() const {
  return root_resource_path_;
}

const TString& AssetExporter::GetRootAssetPath() const {
  return root_asset_path_;
}

void AssetExporter::Process(const AssetExportDescr& export_descr) {
  COMET_LOG_INFO(LoggerType::External, "AssetExporter::Process",
                 "processing asset", "asset_path", export_descr.asset_abs_path);

  auto* allocator{export_descr.allocator};
  auto* asset_export{GenerateAssetExport()};
  asset_export->exporter = this;
  auto& context{asset_export->context};
  context.allocator = allocator;
  context.files = ResourceFiles{allocator};
  auto& descr{context.asset_descr};
  descr.asset_abs_path = export_descr.asset_abs_path;

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
      COMET_LOG_ERROR(LoggerType::External, "AssetExporter::Process",
                      "compression mode is invalid, using none", "asset_path",
                      descr.asset_path, "compression_mode",
                      resource::GetCompressionModeLabel(compression_mode_),
                      "compression_mode_value",
                      ToUnderlying(compression_mode_));

      compression_mode_label = kCometResourceCompressionModeNone.data();
      break;
  }

  descr.metadata[kCometEditorAssetMetadataKeyCompressionMode] =
      compression_mode_label;

  auto job_stack_size{job::JobStackSize::Large};

#ifdef COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
  job_stack_size = job::JobStackSize::ExternalLibrary;
#else
  COMET_LOG_WARNING(LoggerType::External, "AssetExporter::Process",
                    "external library support is disabled");
#endif  // COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT

#ifdef COMET_FIBER_DEBUG_LABEL
  schar debug_label[fiber::Fiber::kDebugLabelMaxLen_ + 1]{'\0'};
#endif  // COMET_FIBER_DEBUG_LABEL

  job::Scheduler::Get().Kick(job::GenerateJobDescr(
      job::JobPriority::Normal, OnResourceFilesProcess, asset_export,
      job_stack_size, export_descr.global_counter,
      COMET_ASSET_HANDLE_FIBER_DEBUG_LABEL(export_descr.asset_abs_path,
                                           debug_label,
                                           fiber::Fiber::kDebugLabelMaxLen_)));
}

void AssetExporter::OnResourceFilesProcess(job::JobParamsHandle params_handle) {
  auto* asset_export{reinterpret_cast<AssetExport*>(params_handle)};
  COMET_ASSERT(asset_export != nullptr, "AssetExporter::OnResourceFilesProcess",
               "asset export is null");

  auto& context{asset_export->context};
  auto& descr{context.asset_descr};
  asset_export->exporter->PopulateFiles(context);

  if (context.files.IsEmpty()) {
    COMET_LOG_ERROR(LoggerType::External,
                    "AssetExporter::OnResourceFilesProcess",
                    "asset processing produced no resource files", "asset_path",
                    descr.asset_abs_path);
    asset_export->exporter->OnAssetProcessed(asset_export);
    return;
  }

  // Every time we get an object, we must use assignment to prevent a bug with
  // GCC where the generated type is an array which... contains an array (which
  // is wrong).
  auto resource_files = nlohmann::json::array();
  constexpr auto kBufferSize{GetCharCount<resource::RawResourceId>() + 1};
  schar buffer[kBufferSize]{'\0'};
  usize out_len{0};

  for (const auto& resource_file : context.files) {
    comet::ConvertToStr(resource_file.resource_id, buffer, kBufferSize,
                        &out_len);
    resource_files.push_back(std::string{buffer, out_len});
  }

  context.asset_descr.metadata[kCometEditorAssetMetadataKeyResourceFiles] =
      resource_files;

  job::Scheduler::Get().Kick(job::GenerateIOJobDescr(
      OnResourceFilesWrite, asset_export, context.global_counter));
}

void AssetExporter::OnResourceFilesWrite(job::IOJobParamsHandle params_handle) {
  auto* asset_export{reinterpret_cast<AssetExport*>(params_handle)};
  COMET_ASSERT(asset_export != nullptr, "AssetExporter::OnResourceFilesWrite",
               "asset export is null");

  auto& context{asset_export->context};
  auto& descr{context.asset_descr};
  COMET_LOG_INFO(LoggerType::External, "AssetExporter::OnResourceFilesWrite",
                 "saving processed asset", "asset_path", descr.asset_abs_path);

  for (const auto& resource_file : context.files) {
    if (!resource::SaveResourceFile(
            GenerateResourcePath(asset_export->exporter->root_resource_path_,
                                 resource_file.resource_id),
            resource_file)) {
      COMET_LOG_ERROR(
          LoggerType::External, "AssetExporter::OnResourceFilesWrite",
          "resource file save failed", "resource_id", resource_file.resource_id,
          "asset_path", descr.asset_abs_path);
    } else {
      COMET_LOG_DEBUG(
          LoggerType::External, "AssetExporter::OnResourceFilesWrite",
          "resource file saved", "resource_id", resource_file.resource_id,
          "asset_path", descr.asset_abs_path);
    }
  }

  SaveMetadata(descr.metadata_path, descr.metadata);
  asset_export->exporter->OnAssetProcessed(asset_export);
}

void AssetExporter::OnAssetProcessed(AssetExport* asset_export) {
  if (asset_export == nullptr) {
    COMET_LOG_ERROR(LoggerType::External, "AssetExporter::OnAssetProcessed",
                    "asset export is null");
    return;
  }

  COMET_LOG_INFO(LoggerType::External, "AssetExporter::OnAssetProcessed",
                 "asset processed", "asset_path",
                 asset_export->context.asset_descr.asset_abs_path);
  DestroyAssetExport(asset_export);
}

AssetExport* AssetExporter::GenerateAssetExport() {
  return asset_export_allocator_.AllocateOneAndPopulate<AssetExport>();
}

void AssetExporter::DestroyAssetExport(AssetExport* asset_export) {
  return asset_export_allocator_.Deallocate(asset_export);
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
