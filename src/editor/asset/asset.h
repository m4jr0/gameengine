// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_ASSET_H_
#define COMET_EDITOR_ASSET_ASSET_H_

#include "comet_precompile.h"

#include "nlohmann/json.hpp"

#include "comet/core/file_system.h"
#include "comet/core/type/tstring.h"
#include "comet/resource/resource.h"

using namespace std::literals;

namespace comet {
namespace editor {
namespace asset {
constexpr unsigned short kCometEditorAssetMetadataIndent{2};
static constexpr auto kCometEditorAssetMetadataFileExtension{
    COMET_CTSTRING_VIEW("meta")};
static constexpr auto kCometEditorAssetFolderMetadataFileExtension{
    COMET_CTSTRING_VIEW("dir.meta")};

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
  TString asset_abs_path{};
  TString asset_path{};
  TString metadata_path{};
  nlohmann::json metadata{};
};
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_ASSET_H_
