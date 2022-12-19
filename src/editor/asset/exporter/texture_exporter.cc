// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "texture_exporter.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

#include "comet/core/engine.h"
#include "comet/resource/resource.h"
#include "comet/resource/texture_resource.h"
#include "comet/utils/file_system.h"
#include "editor/asset/asset.h"

namespace comet {
namespace editor {
namespace asset {
bool TextureExporter::IsCompatible(std::string_view extension) const {
  return extension == "png" || extension == "jpg";
}

std::vector<resource::ResourceFile> TextureExporter::GetResourceFiles(
    AssetDescr& asset_descr) const {
  std::vector<resource::ResourceFile> resource_files{};
  s32 tex_width{0};
  s32 tex_height{0};
  s32 tex_channels{0};
  auto* pixel_data{stbi_load(asset_descr.asset_abs_path.c_str(), &tex_width,
                             &tex_height, &tex_channels, STBI_rgb_alpha)};

  if (pixel_data == nullptr) {
    COMET_LOG_GLOBAL_ERROR("Failed to load texture image");
    return resource_files;
  }

  resource::TextureResource texture{};
  texture.id = resource::GenerateResourceIdFromPath(asset_descr.asset_path);
  texture.type_id = resource::TextureResource::kResourceTypeId;
  texture.descr.size = static_cast<comet::u64>(tex_width) * tex_height * 4;
  texture.descr.resolution[0] = tex_width;
  texture.descr.resolution[1] = tex_height;
  texture.descr.resolution[2] = 0;
  texture.descr.channel_count = tex_channels;
  texture.descr.format = rendering::TextureFormat::Rgba8;
  texture.data = {pixel_data, pixel_data + texture.descr.size};

  asset_descr.metadata[kCometEditorTextureMetadataKeyFormat] =
      kCometEditorTextureFormatRgba8;
  asset_descr.metadata[kCometEditorTextureMetadataKeyWidth] =
      texture.descr.resolution[0];
  asset_descr.metadata[kCometEditorTextureMetadataKeyHeight] =
      texture.descr.resolution[1];
  asset_descr.metadata[kCometEditorTextureMetadataKeySize] = texture.descr.size;

  resource_files.push_back(Engine::Get().GetResourceManager().GetResourceFile(
      texture, compression_mode_));
  stbi_image_free(pixel_data);
  return resource_files;
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
