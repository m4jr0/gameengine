// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_TEXTURE_RESOURCE_H_
#define COMET_COMET_RESOURCE_TEXTURE_RESOURCE_H_

#include "comet_precompile.h"

#include "comet/resource/resource.h"

namespace comet {
namespace resource {
namespace texture {
enum class TextureType : u32 {
  Unknown = 0,
  Ambient,
  Diffuse,
  Specular,
  Height
};

enum class TextureFormat : u32 { Unknown = 0, Rgba8, Rgb8 };

struct TextureResourceDescr {
  u64 size{0};
  TextureFormat format{TextureFormat::Unknown};
  u32 resolution[3]{0, 0, 0};
  u8 channel_number{0};
};

struct TextureResource : public Resource {
  static const ResourceTypeId kResourceTypeId;

  TextureResourceDescr descr;
  std::vector<char> data;
};

class TextureHandler : public ResourceHandler {
 public:
  TextureHandler() = default;
  TextureHandler(const TextureHandler&) = delete;
  TextureHandler(TextureHandler&&) = delete;
  TextureHandler& operator=(const TextureHandler&) = delete;
  TextureHandler& operator=(TextureHandler&&) = delete;
  ~TextureHandler() = default;

 protected:
  ResourceFile Pack(const Resource& resource,
                    CompressionMode compression_mode) const override;
  std::unique_ptr<Resource> Unpack(const ResourceFile& file) const override;
};
}  // namespace texture
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_TEXTURE_RESOURCE_H_
