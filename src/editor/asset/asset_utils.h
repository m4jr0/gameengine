// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_ASSET_UTILS_H_
#define COMET_EDITOR_ASSET_ASSET_UTILS_H_

#include "comet_precompile.h"

#include "nlohmann/json.hpp"

namespace comet {
namespace editor {
namespace asset {
std::string GetAssetMetadataFilePath(std::string_view asset_file_path);
void SaveMetadata(const schar* metadata_file_path,
                  const nlohmann::json& metadata);
void SaveMetadata(const std::string& metadata_file_path,
                  const nlohmann::json& metadata);
nlohmann::json GetMetadata(const schar* metadata_file_path);
nlohmann::json GetMetadata(const std::string& metadata_file_path);
nlohmann::json SetAndGetMetadata(const schar* metadata_file_path);
nlohmann::json SetAndGetMetadata(const std::string& metadata_file_path);
bool IsMetadataFile(const std::string& file_path);
bool IsMetadataFile(const schar* file_path);
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_ASSET_UTILS_H_
