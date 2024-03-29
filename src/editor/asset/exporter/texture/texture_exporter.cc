// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "texture_exporter.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "comet/core/file_system.h"
#include "comet/core/generator.h"
#include "comet/resource/resource.h"
#include "comet/resource/resource_manager.h"
#include "comet/resource/texture_resource.h"
#include "editor/asset/asset.h"

namespace comet {
namespace editor {
namespace asset {
bool TextureExporter::IsCompatible(CTStringView extension) const {
  return extension == COMET_TCHAR("png") || extension == COMET_TCHAR("jpg") ||
         extension == COMET_TCHAR("tga");
}

std::vector<resource::ResourceFile> TextureExporter::GetResourceFiles(
    AssetDescr& asset_descr) const {
  std::vector<resource::ResourceFile> resource_files{};
  s32 tex_width{0};
  s32 tex_height{0};
  s32 tex_channels{0};
#ifdef COMET_WIDE_TCHAR
  auto* asset_abs_path{
      GenerateForOneFrame<schar>(asset_descr.asset_abs_path.GetCTStr(),
                                 asset_descr.asset_abs_path.GetLength())};
#else
  auto* asset_abs_path{asset_descr.asset_abs_path.GetCTStr()};
#endif  // COMET_WIDE_TCHAR
  auto* pixel_data{stbi_load(asset_abs_path, &tex_width, &tex_height,
                             &tex_channels, STBI_rgb_alpha)};

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
  texture.descr.channel_count = static_cast<u8>(tex_channels);
  texture.descr.format = rendering::TextureFormat::Rgba8;
  texture.data = {pixel_data, pixel_data + texture.descr.size};

  asset_descr.metadata[kCometEditorTextureMetadataKeyFormat] =
      kCometEditorTextureFormatRgba8;
  asset_descr.metadata[kCometEditorTextureMetadataKeyWidth] =
      texture.descr.resolution[0];
  asset_descr.metadata[kCometEditorTextureMetadataKeyHeight] =
      texture.descr.resolution[1];
  asset_descr.metadata[kCometEditorTextureMetadataKeySize] = texture.descr.size;

  resource_files.push_back(resource::ResourceManager::Get().GetResourceFile(
      texture, compression_mode_));
  stbi_image_free(pixel_data);
  return resource_files;
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
