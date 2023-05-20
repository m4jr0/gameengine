// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONF_CONFIGURATION_VALUE_H_
#define COMET_COMET_CORE_CONF_CONFIGURATION_VALUE_H_

#include "comet_precompile.h"

using namespace std::literals;

namespace comet {
namespace conf {
using ConfKey = stringid::StringId;

// Possible entries.
// Application. //////////////////////////////////////////////////////////
static const ConfKey kApplicationName{COMET_STRING_ID("application_name")};
static const ConfKey kApplicationMajorVersion{
    COMET_STRING_ID("application_major_version")};
static const ConfKey kApplicationMinorVersion{
    COMET_STRING_ID("application_minor_version")};
static const ConfKey kApplicationPatchVersion{
    COMET_STRING_ID("application_patch_version")};

// Core. /////////////////////////////////////////////////////////////////
static const ConfKey kCoreMsPerUpdate{COMET_STRING_ID("core_ms_per_update")};

// Event. ////////////////////////////////////////////////////////////////
static const ConfKey kEventMaxQueueSize{
    COMET_STRING_ID("event_max_queue_size")};

// Rendering. ////////////////////////////////////////////////////////////
// Common.
static const ConfKey kRenderingDriver{COMET_STRING_ID("rendering_driver")};
static const ConfKey kRenderingWindowWidth{
    COMET_STRING_ID("rendering_window_width")};
static const ConfKey kRenderingWindowHeight{
    COMET_STRING_ID("rendering_window_height")};
static const ConfKey kRenderingClearColorR{
    COMET_STRING_ID("rendering_clear_color_r")};
static const ConfKey kRenderingClearColorG{
    COMET_STRING_ID("rendering_clear_color_g")};
static const ConfKey kRenderingClearColorB{
    COMET_STRING_ID("rendering_clear_color_b")};
static const ConfKey kRenderingClearColorA{
    COMET_STRING_ID("rendering_clear_color_a")};
static const ConfKey kRenderingIsVsync{COMET_STRING_ID("rendering_is_vsync")};
static const ConfKey kRenderingIsTripleBuffering{
    COMET_STRING_ID("rendering_is_triple_buffering")};
static const ConfKey kRenderingFpsCap{COMET_STRING_ID("rendering_fps_cap")};
static const ConfKey kRenderingAntiAliasing{
    COMET_STRING_ID("rendering_anti_aliasing")};
static const ConfKey kRenderingIsSamplerAnisotropy{
    COMET_STRING_ID("rendering_is_sampler_anisotropy")};
static const ConfKey kRenderingIsSampleRateShading{
    COMET_STRING_ID("rendering_is_sample_rate_shading")};

static constexpr auto kRenderingAntiAliasingTypeNone{"none"sv};
static constexpr auto kRenderingAntiAliasingTypeMsaaX64{"msaax64"sv};
static constexpr auto kRenderingAntiAliasingTypeMsaaX32{"msaax32"sv};
static constexpr auto kRenderingAntiAliasingTypeMsaaX16{"msaax16"sv};
static constexpr auto kRenderingAntiAliasingTypeMsaaX8{"msaax8"sv};
static constexpr auto kRenderingAntiAliasingTypeMsaaX4{"msaax4"sv};
static constexpr auto kRenderingAntiAliasingTypeMsaaX2{"msaax2"sv};
static constexpr auto kRenderingAntiAliasingTypeMsaa{"msaa"sv};

// OpenGL.
static constexpr auto kRenderingDriverOpengl{"opengl"sv};

static const ConfKey kRenderingOpenGlMajorVersion{
    COMET_STRING_ID("rendering_opengl_major_version")};
static const ConfKey kRenderingOpenGlMinorVersion{
    COMET_STRING_ID("rendering_opengl_minor_version")};

// Vulkan.
static constexpr auto kRenderingDriverVulkan{"vulkan"sv};

static const ConfKey kRenderingVulkanVariantVersion{
    COMET_STRING_ID("rendering_vulkan_variant_version")};
static const ConfKey kRenderingVulkanMajorVersion{
    COMET_STRING_ID("rendering_vulkan_major_version")};
static const ConfKey kRenderingVulkanMinorVersion{
    COMET_STRING_ID("rendering_vulkan_minor_version")};
static const ConfKey kRenderingVulkanPatchVersion{
    COMET_STRING_ID("rendering_vulkan_patch_version")};
static const ConfKey kRenderingVulkanMaxFramesInFlight{
    COMET_STRING_ID("rendering_vulkan_max_frames_in_flight")};

// Direct3D 12.
static constexpr auto kRenderingDriverDirect3d12{"direct3d12"sv};

// Resource. /////////////////////////////////////////////////////////////
static const ConfKey kResourceRootPath{COMET_STRING_ID("resource_root_path")};

constexpr u16 kMaxStrValueLength{260};

union ConfValue {
  schar str[kMaxStrValueLength];
  u8 ubyte;
  u16 ushort;
  u32 u;
  u64 ulong;
  s8 sbyte;
  s16 sshort;
  s32 s;
  s64 slong;
  f32 f;
  f64 flong;
  uindex index;
  ux uarch;
  sx sarch;
  fx farch;
  bool flag;
};

ConfValue GetDefaultValue(const std::string& key);
ConfValue GetDefaultValue(const schar* key);
ConfValue GetDefaultValue(ConfKey key);
}  // namespace conf
}  // namespace comet

#endif  // COMET_COMET_CORE_CONF_CONFIGURATION_VALUE_H_