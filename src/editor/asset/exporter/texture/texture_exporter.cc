// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "texture_exporter.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/generator.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"
#include "comet/rendering/rendering_common.h"
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

void TextureExporter::PopulateFiles(ResourceFilesContext& context) const {
  auto& asset_descr{context.asset_descr};
  auto& resource_files{context.files};

  TextureContext texture_context{};
#ifdef COMET_WIDE_TCHAR
  texture_context.path =
      GenerateForOneFrame<schar>(asset_descr.asset_abs_path.GetCTStr(),
                                 asset_descr.asset_abs_path.GetLength());
#else
  texture_context.path = asset_descr.asset_abs_path.GetCTStr();
#endif  // COMET_WIDE_TCHAR

  {
    job::CounterGuard guard{};

    job::Scheduler::Get().KickAndWait(job::GenerateIOJobDescr(
        OnTextureLoading, &texture_context, guard.GetCounter()));
  }

  if (texture_context.pixel_data == nullptr) {
    COMET_LOG_GLOBAL_ERROR("Failed to load texture image");
    return;
  }

  COMET_LOG_GLOBAL_DEBUG("Processing texture at ", texture_context.path, "...");
  resource::TextureResource texture{};
  texture.id = resource::GenerateResourceIdFromPath<resource::TextureResource>(
      asset_descr.asset_path);
  texture.type_id = resource::TextureResource::kResourceTypeId;
  texture.descr.size = static_cast<comet::u64>(texture_context.tex_width) *
                       texture_context.tex_height * 4;
  texture.descr.resolution[0] = texture_context.tex_width;
  texture.descr.resolution[1] = texture_context.tex_height;
  texture.descr.resolution[2] = 0;
  texture.descr.channel_count = static_cast<u8>(texture_context.tex_channels);
  texture.descr.format = rendering::TextureFormat::Rgba8;

  texture.data = Array<u8>{context.allocator};
  texture.data.Resize(texture.descr.size);
  memory::CopyMemory(texture.data.GetData(), texture_context.pixel_data,
                     texture.descr.size);

  asset_descr.metadata[kCometEditorTextureMetadataKeyFormat] =
      kCometEditorTextureFormatRgba8;
  asset_descr.metadata[kCometEditorTextureMetadataKeyWidth] =
      texture.descr.resolution[0];
  asset_descr.metadata[kCometEditorTextureMetadataKeyHeight] =
      texture.descr.resolution[1];
  asset_descr.metadata[kCometEditorTextureMetadataKeySize] = texture.descr.size;

  resource_files.PushBack(resource::ResourceManager::Get().GetTextures()->Pack(
      texture, compression_mode_));
  stbi_image_free(texture_context.pixel_data);
  COMET_LOG_GLOBAL_DEBUG("Texture processed at ", texture_context.path);
}

void TextureExporter::OnTextureLoading(job::IOJobParamsHandle params_handle) {
  auto* texture_context{static_cast<TextureContext*>(params_handle)};
  texture_context->pixel_data =
      stbi_load(texture_context->path, &texture_context->tex_width,
                &texture_context->tex_height, &texture_context->tex_channels,
                STBI_rgb_alpha);
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
