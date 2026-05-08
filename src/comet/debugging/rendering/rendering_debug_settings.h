// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_DEBUG_CAMERA_RENDERING_DEBUG_SETTINGS_H_
#define COMET_COMET_DEBUG_CAMERA_RENDERING_DEBUG_SETTINGS_H_

#include "comet/core/essentials.h"

#ifdef COMET_DEBUG

namespace comet {
namespace debug {
using WorldDebugFlags = u32;

enum WorldDebugFlagBits : WorldDebugFlags {
  kWorldDebugFlagBitsNone = 0x0,

  kWorldDebugFlagBitsDisableTextures = 0x1,
  kWorldDebugFlagBitsDisableLighting = 0x2,
  kWorldDebugFlagBitsDisableShadows = 0x4,
  kWorldDebugFlagBitsShowNormals = 0x8,

  kWorldDebugFlagBitsAll = static_cast<WorldDebugFlags>(-1),
};

using DebugDrawFlags = u32;

enum DebugDrawFlagBits : DebugDrawFlags {
  kDebugDrawFlagBitsNone = 0x0,

  kDebugDrawFlagBitsCullingBoxes = 0x1,
  kDebugDrawFlagBitsCameraFrustums = 0x2,
  kDebugDrawFlagBitsCascadeFrustums = 0x4,
  kDebugDrawFlagBitsLightFrustums = 0x8,

  kDebugDrawFlagBitsAll = static_cast<DebugDrawFlags>(-1),
};

class RenderingDebugSettings {
 public:
  static RenderingDebugSettings& Get();

  bool IsDebugCameraEnabled() const noexcept;
  void SetDebugCameraEnabled(bool value) noexcept;

  WorldDebugFlags GetFlags(bool is_debug_camera) const noexcept;
  void SetFlag(bool is_debug_camera, u32 flag, bool value) noexcept;
  bool HasFlag(bool is_debug_camera, u32 flag) const noexcept;

  DebugDrawFlags GetDebugDrawFlags(bool is_debug_camera) const noexcept;
  void SetDebugDrawFlag(bool is_debug_camera, DebugDrawFlags flag,
                        bool value) noexcept;
  bool HasDebugDrawFlag(bool is_debug_camera,
                        DebugDrawFlags flag) const noexcept;

 private:
  bool is_debug_camera_enabled_{false};
  WorldDebugFlags game_flags_{kWorldDebugFlagBitsNone};
  WorldDebugFlags debug_flags_{kWorldDebugFlagBitsDisableLighting |
                               kWorldDebugFlagBitsDisableShadows};

  DebugDrawFlags game_debug_draw_flags_{kDebugDrawFlagBitsNone};
  DebugDrawFlags debug_debug_draw_flags_{kDebugDrawFlagBitsCameraFrustums};
};
}  // namespace debug
}  // namespace comet

#endif  // COMET_DEBUG

#endif  // COMET_COMET_DEBUG_CAMERA_RENDERING_DEBUG_SETTINGS_H_