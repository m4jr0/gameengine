// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_ASSET_H_
#define COMET_EDITOR_ASSET_ASSET_H_

#include "comet_precompile.h"

#include "nlohmann/json.hpp"

#include "comet/resource/resource.h"

namespace comet {
namespace editor {
namespace asset {
constexpr unsigned short kCometEditorAssetMetadataIndent{2};

constexpr char kCometEditorAssetMetadataFileExtension[]{"meta"};
constexpr char kCometEditorAssetFolderMetadataFileExtension[]{"dir.meta"};

constexpr char kCometResourceCompressionModeNone[]{"none"};
constexpr char kCometResourceCompressionModeLz4[]{"lz4"};

constexpr char kCometEditorAssetCometVersion[]{"comet_version"};
constexpr char kCometEditorAssetMetadataKeyVersion[]{"asset_version"};
constexpr char kCometEditorAssetMetadataKeyCreationTime[]{"creation_time"};
constexpr char kCometEditorAssetMetadataKeyUpdateTime[]{"update_time"};
constexpr char kCometEditorAssetMetadataKeyCompressionMode[]{
    "compression_mode"};

struct AssetDescr {
  std::string asset_abs_path;
  std::string asset_path;
  std::string metadata_path;
  nlohmann::json metadata;
};
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_ASSET_H_
