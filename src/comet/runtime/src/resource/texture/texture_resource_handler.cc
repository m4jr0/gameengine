// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/resource/texture/texture_resource_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/memory_utils.h"
#include "comet/core/container/array.h"
#include "comet/data/render/texture.h"

namespace comet {
namespace resource {
TextureResourceHandler::TextureResourceHandler(
    const ResourceHandlerDescr& descr)
    : Base{descr} {}

ResourceFile TextureResourceHandler::Pack(const TextureResource& resource,
    CompressionMode compression_mode) {
  return PackTextureResource(resource, compression_mode, byte_allocator_);
}

void TextureResourceHandler::Unpack(const ResourceFile& file,
    ResourceLifeSpan life_span, TextureResource* resource) {
  UnpackTextureResource(file, byte_allocator_,
      ResolveAllocator(byte_allocator_, life_span), resource);
  resource->life_span = life_span;
}
TextureResource* TextureResourceHandler::GetDefaultTextureResource() {
  if (default_texture_ == nullptr) {
    constexpr auto kDimension{256};
    constexpr auto kPatternThreshold{kDimension / 16};
    constexpr auto kChannelCount{4};
    constexpr u8 kColor1[]{30, 30, 30, 255};
    constexpr u8 kColor2[]{200, 200, 200, 255};

    default_texture_ = std::make_unique<TextureResource>();
    default_texture_->id = kFallbackRawResourceId;
    default_texture_->type_id = TextureResource::kResourceTypeId;

    auto& descr{default_texture_->descr};

    descr.size = static_cast<u64>(kDimension) * kDimension * kChannelCount;
    COMET_ASSERT(descr.size > 0,
                 "TextureResourceHandler::GetDefaultTextureResource",
                 "default texture size is zero");

    descr.format = render::TextureFormat::Rgba8;
    descr.resolution[0] = kDimension;
    descr.resolution[1] = kDimension;
    descr.channel_count = kChannelCount;

    default_texture_->data = Array<u8>{&resource_data_allocator_};
    auto& data{default_texture_->data};

    data.Resize(descr.size);
    COMET_ASSERT(data.GetSize() == descr.size,
                 "TextureResourceHandler::GetDefaultTextureResource",
                 "default texture data size mismatch", "data_size",
                 data.GetSize(), "expected_size", descr.size);

    auto is_color_1{false};

    for (usize col{0}; col < kDimension; ++col) {
      if (col % kPatternThreshold == 0) {
        is_color_1 = !is_color_1;
      }

      for (usize row{0}; row < kDimension; ++row) {
        const auto row_image_index{col * kDimension + row};
        const auto data_index{kChannelCount * row_image_index};

        if (row_image_index % kPatternThreshold == 0) {
          is_color_1 = !is_color_1;
        }

        const auto& color{is_color_1 ? kColor1 : kColor2};

        for (usize k{0}; k < kChannelCount; ++k) {
          data[data_index + k] = color[k];
        }
      }
    }
  }

  return default_texture_.get();
}

TextureResource* TextureResourceHandler::GetDefaultDiffuseTextureResource() {
  if (diffuse_texture_ == nullptr) {
    constexpr auto kDimension{16};
    constexpr auto kChannelCount{4};
    constexpr u8 kColor[]{150, 150, 150, 255};

    diffuse_texture_ = std::make_unique<TextureResource>();
    diffuse_texture_->id = kDefaultDiffuseTextureId.GetValue();
    diffuse_texture_->type_id = TextureResource::kResourceTypeId;

    auto& descr{diffuse_texture_->descr};

    descr.size = static_cast<u64>(kDimension) * kDimension * kChannelCount;
    COMET_ASSERT(descr.size > 0,
                 "TextureResourceHandler::GetDefaultDiffuseTextureResource",
                 "default diffuse texture size is zero");

    descr.format = render::TextureFormat::Rgba8;
    descr.resolution[0] = kDimension;
    descr.resolution[1] = kDimension;
    descr.channel_count = kChannelCount;

    diffuse_texture_->data = Array<u8>{&resource_data_allocator_};
    auto& data{diffuse_texture_->data};

    data.Resize(descr.size);
    COMET_ASSERT(data.GetSize() == descr.size,
                 "TextureResourceHandler::GetDefaultDiffuseTextureResource",
                 "default diffuse texture data size mismatch", "data_size",
                 data.GetSize(), "expected_size", descr.size);

    for (usize i{0}; i < descr.size; ++i) {
      data[i] = kColor[i % kChannelCount];
    }
  }

  return diffuse_texture_.get();
}

TextureResource* TextureResourceHandler::GetDefaultSpecularTextureResource() {
  if (specular_texture_ == nullptr) {
    constexpr auto kDimension{1};
    constexpr auto kChannelCount{4};
    constexpr u8 kColor[]{0, 0, 0, 255};

    specular_texture_ = std::make_unique<TextureResource>();
    specular_texture_->id = kDefaultSpecularTextureId.GetValue();
    specular_texture_->type_id = TextureResource::kResourceTypeId;

    auto& descr{specular_texture_->descr};

    descr.size = static_cast<u64>(kDimension) * kDimension * kChannelCount;
    COMET_ASSERT(descr.size > 0,
                 "TextureResourceHandler::GetDefaultSpecularTextureResource",
                 "default specular texture size is zero");

    descr.format = render::TextureFormat::Rgba8;
    descr.resolution[0] = kDimension;
    descr.resolution[1] = kDimension;
    descr.channel_count = kChannelCount;

    specular_texture_->data = Array<u8>{&resource_data_allocator_};
    auto& data{specular_texture_->data};

    data.Resize(descr.size);
    COMET_ASSERT(data.GetSize() == descr.size,
                 "TextureResourceHandler::GetDefaultSpecularTextureResource",
                 "default specular texture data size mismatch", "data_size",
                 data.GetSize(), "expected_size", descr.size);

    for (usize i{0}; i < descr.size; ++i) {
      data[i] = kColor[i % kChannelCount];
    }
  }

  return specular_texture_.get();
}

TextureResource* TextureResourceHandler::GetDefaultNormalTextureResource() {
  if (normal_texture_ == nullptr) {
    constexpr auto kDimension{1};
    constexpr auto kChannelCount{4};
    constexpr u8 kColor[]{128, 128, 255, 255};

    normal_texture_ = std::make_unique<TextureResource>();
    normal_texture_->id = kDefaultNormalTextureId.GetValue();
    normal_texture_->type_id = TextureResource::kResourceTypeId;

    auto& descr{normal_texture_->descr};

    descr.size = static_cast<u64>(kDimension) * kDimension * kChannelCount;
    COMET_ASSERT(descr.size > 0,
                 "TextureResourceHandler::GetDefaultNormalTextureResource",
                 "default normal texture size is zero");

    descr.format = render::TextureFormat::Rgba8;
    descr.resolution[0] = kDimension;
    descr.resolution[1] = kDimension;
    descr.channel_count = kChannelCount;

    normal_texture_->data = Array<u8>{&resource_data_allocator_};
    auto& data{normal_texture_->data};

    data.Resize(descr.size);
    COMET_ASSERT(data.GetSize() == descr.size,
                 "TextureResourceHandler::GetDefaultNormalTextureResource",
                 "default normal texture data size mismatch", "data_size",
                 data.GetSize(), "expected_size", descr.size);

    for (usize i{0}; i < descr.size; ++i) {
      data[i] = kColor[i % kChannelCount];
    }
  }

  return normal_texture_.get();
}

void TextureResourceHandler::InitializeDefaults() {
  COMET_ASSERT(defaults_.IsEmpty(),
               "TextureResourceHandler::InitializeDefaults",
               "default resource list is not empty");

  defaults_.Reserve(4);
  RegisterDefaultResource(GetDefaultTextureResource());
  RegisterDefaultResource(GetDefaultDiffuseTextureResource());
  RegisterDefaultResource(GetDefaultSpecularTextureResource());
  RegisterDefaultResource(GetDefaultNormalTextureResource());
}

void TextureResourceHandler::DestroyDefaults() {
  default_texture_.reset();
  diffuse_texture_.reset();
  specular_texture_.reset();
  normal_texture_.reset();
}
}  // namespace resource
}  // namespace comet