// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_TYPE_TEXTURE_H_
#define COMET_COMET_RENDERING_TYPE_TEXTURE_H_

#include "comet/core/essentials.h"
#include "comet/math/vector.h"

namespace comet {
namespace rendering {
constexpr math::Vec3 kColorBlackRgb{.0f, .0f, .0f};
constexpr math::Vec3 kColorWhiteRgb{1.0f, 1.0f, 1.0f};
constexpr math::Vec3 kColorRedRgb{1.0f, .0f, .0f};
constexpr math::Vec3 kColorGreenRgb{.0f, 1.0f, .0f};
constexpr math::Vec3 kColorBlueRgb{.0f, .0f, 1.0f};
constexpr math::Vec3 kColorYellowRgb{1.0f, 1.0f, .0f};
constexpr math::Vec3 kColorCyanRgb{.0f, 1.0f, 1.0f};
constexpr math::Vec3 kColorMagentaRgb{1.0f, .0f, 1.0f};

constexpr math::Vec4 kColorBlackRgba{kColorBlackRgb, 1.0f};
constexpr math::Vec4 kColorWhiteRgba{kColorWhiteRgb, 1.0f};
constexpr math::Vec4 kColorRedRgba{kColorRedRgb, 1.0f};
constexpr math::Vec4 kColorGreenRgba{kColorGreenRgb, 1.0f};
constexpr math::Vec4 kColorBlueRgba{kColorBlueRgb, 1.0f};
constexpr math::Vec4 kColorYellowRgba{kColorYellowRgb, 1.0f};
constexpr math::Vec4 kColorCyanRgba{kColorCyanRgb, 1.0f};
constexpr math::Vec4 kColorMagentaRgba{kColorMagentaRgb, 1.0f};

enum class TextureType : u8 {
  Unknown = 0,
  Ambient,
  Diffuse,
  Specular,
  Normal,
  Color
};

enum class TextureRepeatMode : u8 {
  Unknown = 0,
  Repeat,
  MirroredRepeat,
  ClampToEdge,
  ClampToBorder
};

enum class TextureFilterMode : u8 { Unknown = 0, Nearest, Linear };

enum class TextureFormat : u32 { Unknown = 0, Rgba8, Rgb8 };
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_TYPE_TEXTURE_H_