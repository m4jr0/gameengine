// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "asset_utils.h"

namespace comet {
namespace editor {
namespace asset {
bool IsMetadataFile(const std::string& file_path) {
  return IsMetadataFile(file_path.c_str());
}

bool IsMetadataFile(const char* file_path) {
  return utils::filesystem::GetExtension(file_path) ==
         kCometEditorAssetMetadataFileExtension;
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
