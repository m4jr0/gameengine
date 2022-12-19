// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "texture_resource.h"

namespace comet {
namespace resource {
const ResourceTypeId TextureResource::kResourceTypeId{
    COMET_STRING_ID("texture")};

ResourceFile TextureHandler::Pack(const Resource& resource,
                                  CompressionMode compression_mode) const {
  const auto& texture{static_cast<const TextureResource&>(resource)};
  ResourceFile file{};
  file.resource_id = texture.id;
  file.resource_type_id = TextureResource::kResourceTypeId;
  file.compression_mode = compression_mode;

  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  const auto data_size{sizeof(u8) * texture.data.size()};

  std::vector<u8> data(kResourceIdSize + kResourceTypeIdSize + data_size);
  uindex cursor{0};
  auto* buffer{data.data()};

  std::memcpy(&buffer[cursor], &texture.id, kResourceIdSize);
  cursor += kResourceIdSize;

  std::memcpy(&buffer[cursor], &texture.type_id, kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  std::memcpy(&buffer[cursor], texture.data.data(), data_size);
  cursor += data_size;

  PackPodResourceDescr(texture.descr, file);
  PackResourceData(data, file);
  return file;
}

std::unique_ptr<Resource> TextureHandler::Unpack(
    const ResourceFile& file) const {
  TextureResource texture{};
  texture.descr = UnpackPodResourceDescr<TextureResourceDescr>(file);
  const auto data{UnpackResourceData(file)};

  const auto* buffer{data.data()};
  uindex cursor{0};
  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  const auto data_size{sizeof(u8) * data.size() - kResourceIdSize -
                       kResourceTypeIdSize};

  std::memcpy(&texture.id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  std::memcpy(&texture.type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  texture.data.resize(data_size);
  std::memcpy(texture.data.data(), &buffer[cursor], data_size);
  cursor += data_size;

  return std::make_unique<TextureResource>(std::move(texture));
}

const Resource* TextureHandler::Get(ResourceId resource_id) {
  if (resource_id == kFlatTextureResourceId) {
    return GetFlatTexture();
  }

  return cache_.Get(resource_id);
}

const Resource* TextureHandler::GetDefaultResource() {
  if (default_texture_ == nullptr) {
    constexpr auto kDimension{256};
    constexpr auto kPatternThreshold{kDimension / 16};
    constexpr auto kChannelCount{4};
    constexpr u8 kColor1[]{30, 30, 30, 255};
    constexpr u8 kColor2[]{200, 200, 200, 255};

    default_texture_ = std::make_unique<TextureResource>();
    default_texture_->id = kDefaultResourceId;
    default_texture_->type_id = TextureResource::kResourceTypeId;

    auto& descr{default_texture_->descr};
    descr.size =
        static_cast<comet::u64>(kDimension) * kDimension * kChannelCount;
    descr.format = rendering::TextureFormat::Rgba8;
    descr.resolution[0] = kDimension;
    descr.resolution[1] = kDimension;
    descr.channel_count = kChannelCount;

    auto& data{default_texture_->data};
    data.resize(descr.size);
    auto is_color_1{false};

    for (uindex col{0}; col < kDimension; ++col) {
      if (col % kPatternThreshold == 0) {
        is_color_1 = !is_color_1;
      }

      for (uindex row{0}; row < kDimension; ++row) {
        const auto row_image_index{col * kDimension + row};
        const auto data_index{kChannelCount * row_image_index};

        if (row_image_index % kPatternThreshold == 0) {
          is_color_1 = !is_color_1;
        }

        const auto& color{is_color_1 ? kColor1 : kColor2};

        for (uindex k{0}; k < kChannelCount; ++k) {
          data[data_index + k] = color[k];
        }
      }
    }
  }

  return default_texture_.get();
}

const Resource* TextureHandler::GetFlatTexture() {
  if (flat_texture_ == nullptr) {
    constexpr auto kDimension{16};
    constexpr auto kChannelCount{4};
    constexpr u8 kColor[]{150, 150, 150, 255};

    flat_texture_ = std::make_unique<TextureResource>();
    flat_texture_->id = kFlatTextureResourceId;
    flat_texture_->type_id = TextureResource::kResourceTypeId;

    auto& descr{flat_texture_->descr};
    descr.size =
        static_cast<comet::u64>(kDimension) * kDimension * kChannelCount;
    descr.format = rendering::TextureFormat::Rgba8;
    descr.resolution[0] = kDimension;
    descr.resolution[1] = kDimension;
    descr.channel_count = kChannelCount;

    auto& data{flat_texture_->data};
    data.resize(descr.size);
    auto is_color_1{false};

    for (uindex i{0}; i < descr.size; ++i) {
      data[i] = kColor[i % kChannelCount];
    }
  }

  return flat_texture_.get();
}
}  // namespace resource
}  // namespace comet
