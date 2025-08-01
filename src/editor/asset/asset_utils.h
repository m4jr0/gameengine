// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_ASSET_UTILS_H_
#define COMET_EDITOR_ASSET_ASSET_UTILS_H_

#include "nlohmann/json.hpp"

#include "comet/core/essentials.h"
#include "comet/core/logger.h"
#include "comet/core/type/array.h"
#include "comet/core/type/tstring.h"
#include "comet/resource/resource.h"

namespace comet {
namespace editor {
namespace asset {
TString GenerateAssetMetadataFilePath(CTStringView asset_file_path);
void SaveMetadata(CTStringView metadata_file_path,
                  const nlohmann::json& metadata);
nlohmann::json GetMetadata(CTStringView metadata_file_path);
nlohmann::json SetAndGetMetadata(CTStringView metadata_file_path);
bool IsMetadataFile(CTStringView file_path);
TString GenerateResourcePath(CTStringView folder_path,
                             resource::ResourceId resource_id);

namespace internal {
schar* GenerateTmpAssetFiberDebugLabel(CTStringView path, schar* buffer,
                                       usize buffer_len);
}  // namespace internal
}  // namespace asset
}  // namespace editor
}  // namespace comet

namespace nlohmann {
template <typename T>
comet::Array<T> json_to_array(const json& json_array,
                              comet::memory::Allocator* allocator) {
  comet::Array<T> array{allocator};

  if (!json_array.is_array()) {
    COMET_LOG_GLOBAL_ERROR("Wrong type found for JSON array: ",
                           json_array.type_name(), "! Ignoring.");
    return array;
  }

  for (const auto& entry : json_array) {
    array.PushBack(entry.get<T>());
  }

  return array;
}

template <typename T>
struct adl_serializer<comet::Array<T>> {
  static comet::Array<T> from_json(const json& json_array,
                                   comet::memory::Allocator* allocator) {
    return json_to_array<T>(json_array, allocator);
  }
};
}  // namespace nlohmann

#ifdef COMET_FIBER_DEBUG_LABEL
#define COMET_ASSET_HANDLE_FIBER_DEBUG_LABEL(path, buffer, buffer_len) \
  comet::editor::asset::internal::GenerateTmpAssetFiberDebugLabel(     \
      path, buffer, buffer_len)
#else
#define COMET_ASSET_HANDLE_FIBER_DEBUG_LABEL(path, buffer, buffer_len) nullptr
#endif  // COMET_FIBER_DEBUG_LABEL

#endif  // COMET_EDITOR_ASSET_ASSET_UTILS_H_
