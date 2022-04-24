// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "texture_exporter.h"

#define STB_IMAGE_IMPLEMENTATION

#include "nlohmann/json.hpp"
#include "stb_image.h"

#include "comet/core/engine.h"
#include "comet/resource/resource.h"
#include "comet/utils/file_system.h"
#include "editor/asset/asset.h"

namespace comet {
namespace editor {
namespace asset {
namespace texture {
bool TextureExporter::AttachResourceToAssetDescr(AssetDescr& asset_descr) {
  s32 tex_width{0};
  s32 tex_height{0};
  s32 tex_channels{0};
  auto* pixel_data{stbi_load(asset_descr.asset_abs_path.c_str(), &tex_width,
                             &tex_height, &tex_channels, STBI_rgb_alpha)};

  if (pixel_data == nullptr) {
    COMET_LOG_GLOBAL_ERROR("Failed to load texture image");
    return false;
  }

  resource::texture::TextureResource texture{};
  texture.id = asset_descr.resource_id;
  texture.type_id = resource::texture::TextureResource::kResourceTypeId;
  texture.descr.size = static_cast<comet::u64>(tex_width) * tex_height * 4;
  texture.descr.resolution[0] = tex_width;
  texture.descr.resolution[1] = tex_height;
  texture.descr.resolution[2] = 0;
  texture.descr.channel_number = tex_channels;
  texture.descr.format = resource::texture::TextureFormat::Rgba8;

  texture.data = {pixel_data, pixel_data + texture.descr.size};

  asset_descr.metadata[kCometEditorTextureMetadataKeyFormat] =
      kCometEditorTextureFormatRgba8;
  asset_descr.metadata[kCometEditorTextureMetadataKeyWidth] =
      texture.descr.resolution[0];
  asset_descr.metadata[kCometEditorTextureMetadataKeyHeight] =
      texture.descr.resolution[1];
  asset_descr.metadata[kCometEditorTextureMetadataKeySize] = texture.descr.size;

  asset_descr.resource = Engine::Get().GetResourceManager().GetResourceFile(
      texture, compression_mode_);

  stbi_image_free(pixel_data);

  return true;
}

bool TextureExporter::IsCompatible(const std::string& extension) {
  return extension == "png" || extension == "jpg";
}
}  // namespace texture
}  // namespace asset
}  // namespace editor
}  // namespace comet
