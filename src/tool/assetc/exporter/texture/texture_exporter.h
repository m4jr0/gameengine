// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_TEXTURE_TEXTURE_EXPORTER_H_
#define COMET_EDITOR_ASSET_EXPORTER_TEXTURE_TEXTURE_EXPORTER_H_

#include "comet/core/concurrency/job/job.h"
#include "comet/core/essentials.h"
#include "comet/core/type/tstring.h"
#include "editor/asset/exporter/asset_exporter.h"

namespace comet {
namespace editor {
namespace asset {
class TextureExporter : public AssetExporter {
 public:
  TextureExporter() = default;
  TextureExporter(const TextureExporter&) = delete;
  TextureExporter(TextureExporter&&) = delete;
  TextureExporter& operator=(const TextureExporter&) = delete;
  TextureExporter& operator=(TextureExporter&&) = delete;
  ~TextureExporter() override = default;

  bool IsCompatible(CTStringView extension) const override;

 protected:
  void PopulateFiles(ResourceFilesContext& context) const override;

 private:
  struct TextureContext {
    s32 tex_width{0};
    s32 tex_height{0};
    s32 tex_channels{0};
    u8* pixel_data{nullptr};
    const schar* path{nullptr};
  };

  static void OnTextureLoading(job::IOJobParamsHandle params_handle);
};
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_TEXTURE_TEXTURE_EXPORTER_H_
