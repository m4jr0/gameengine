// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_render_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/debug/rendering_debug_settings.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_DEBUG

namespace comet {
namespace debug {
RenderingDebugSettings& RenderingDebugSettings::Get() {
  static RenderingDebugSettings singleton{};
  return singleton;
}

bool RenderingDebugSettings::IsDebugCameraEnabled() const noexcept {
  return is_debug_camera_enabled_;
}

void RenderingDebugSettings::SetDebugCameraEnabled(bool value) noexcept {
  is_debug_camera_enabled_ = value;
}

WorldDebugFlags RenderingDebugSettings::GetFlags(
    bool is_debug_camera) const noexcept {
  return is_debug_camera ? debug_flags_ : game_flags_;
}

void RenderingDebugSettings::SetFlag(bool is_debug_camera, u32 flag,
                                     bool value) noexcept {
  auto& flags = is_debug_camera ? debug_flags_ : game_flags_;

  if (value) {
    flags |= flag;
  } else {
    flags &= ~flag;
  }
}

bool RenderingDebugSettings::HasFlag(bool is_debug_camera,
                                     u32 flag) const noexcept {
  const auto flags = is_debug_camera ? debug_flags_ : game_flags_;
  return (flags & flag) != 0;
}

DebugDrawFlags RenderingDebugSettings::GetDebugDrawFlags(
    bool is_debug_camera) const noexcept {
  return is_debug_camera ? debug_debug_draw_flags_ : game_debug_draw_flags_;
}

void RenderingDebugSettings::SetDebugDrawFlag(bool is_debug_camera,
                                              DebugDrawFlags flag,
                                              bool value) noexcept {
  auto& flags =
      is_debug_camera ? debug_debug_draw_flags_ : game_debug_draw_flags_;

  if (value) {
    flags |= flag;
  } else {
    flags &= ~flag;
  }
}

bool RenderingDebugSettings::HasDebugDrawFlag(
    bool is_debug_camera, DebugDrawFlags flag) const noexcept {
  return (GetDebugDrawFlags(is_debug_camera) & flag) != 0;
}
}  // namespace debug
}  // namespace comet

#endif  // COMET_DEBUG