// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_ASSET_H_
#define COMET_EDITOR_ASSET_ASSET_H_

#include "comet_precompile.h"

#include "nlohmann/json.hpp"

#include "comet/resource/resource.h"

using namespace std::literals;

namespace comet {
namespace editor {
namespace asset {
constexpr unsigned short kCometEditorAssetMetadataIndent{2};

static constexpr auto kCometEditorAssetMetadataFileExtension{"meta"sv};
static constexpr auto kCometEditorAssetFolderMetadataFileExtension{
    "dir.meta"sv};

static constexpr auto kCometResourceCompressionModeNone{"none"sv};
static constexpr auto kCometResourceCompressionModeLz4{"lz4"sv};

static constexpr auto kCometEditorAssetCometVersion{"comet_version"sv};
static constexpr auto kCometEditorAssetMetadataKeyVersion{"asset_version"sv};
static constexpr auto kCometEditorAssetMetadataKeyCreationTime{
    "creation_time"sv};
static constexpr auto kCometEditorAssetMetadataKeyUpdateTime{"update_time"sv};
static constexpr auto kCometEditorAssetMetadataKeyCompressionMode{
    "compression_mode"sv};

struct AssetDescr {
  std::string asset_abs_path{};
  std::string asset_path{};
  std::string metadata_path{};
  nlohmann::json metadata{};
};
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_ASSET_H_
