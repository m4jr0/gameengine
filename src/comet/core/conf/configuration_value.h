// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_CONF_CONFIGURATION_VALUE_H_
#define COMET_COMET_CORE_CONF_CONFIGURATION_VALUE_H_

#include "comet/core/essentials.h"
#include "comet/core/type/string_id.h"

using namespace std::literals;

namespace comet {
namespace conf {
using ConfKey = stringid::StringId;

constexpr auto kMaxKeyLength{64};
constexpr auto kMaxValueLength{256};
constexpr auto kMaxLineLength{512};

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
static const ConfKey kCoreForcedFiberWorkerCount{
    COMET_STRING_ID("core_forced_fiber_worker_count")};
static const ConfKey kCoreForcedIOWorkerCount{
    COMET_STRING_ID("core_forced_io_worker_count")};
static const ConfKey kCoreLargeFiberCount{
    COMET_STRING_ID("core_large_fiber_count")};
static const ConfKey kCoreGiganticFiberCount{
    COMET_STRING_ID("core_gigantic_fiber_count")};
static const ConfKey kCoreExternalLibraryFiberCount{
    COMET_STRING_ID("core_external_library_fiber_count")};
static const ConfKey kCoreJobCounterCount{
    COMET_STRING_ID("core_job_counter_count")};
static const ConfKey kCoreJobQueueCount{
    COMET_STRING_ID("core_job_queue_count")};
static const ConfKey kCoreIsMainThreadWorkerDisabled{
    COMET_STRING_ID("core_is_main_thread_worker_disabled")};
static const ConfKey kCoreTaggedHeapCapacity{
    COMET_STRING_ID("core_tagged_heap_capacity")};
static const ConfKey kCoreFiberFrameAllocatorBaseCapacity{
    COMET_STRING_ID("core_fiber_frame_allocator_base_capacity")};
static const ConfKey kCoreIOFrameAllocatorBaseCapacity{
    COMET_STRING_ID("core_io_frame_allocator_base_capacity")};
static const ConfKey kCoreTStringAllocatorCapacity{
    COMET_STRING_ID("core_tstring_allocator_capacity")};

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

// Other.
#ifdef COMET_DEBUG
static constexpr auto kRenderingDriverEmpty{"empty"sv};
#endif  // COMET_DEBUG

// Resource. /////////////////////////////////////////////////////////////
static const ConfKey kResourceRootPath{COMET_STRING_ID("resource_root_path")};

constexpr u16 kMaxStrValueLength{260};

union ConfValue {
  schar str_value[kMaxStrValueLength];
  wchar wstr_value[kMaxStrValueLength * sizeof(wchar)];
  u8 u8_value;
  u16 u16_value;
  u32 u32_value;
  u64 u64_value;
  s8 s8_value;
  s16 s16_value;
  s32 s32_value;
  s64 s64_value;
  f32 f32_value;
  f64 f64_value;
  usize uindex_value;
  ux ux_value;
  sx sx_value;
  fx fx_value;
  bool bool_value;
};

ConfValue GetDefaultValue(const schar* key);
ConfValue GetDefaultValue(ConfKey key);
}  // namespace conf
}  // namespace comet

#endif  // COMET_COMET_CORE_CONF_CONFIGURATION_VALUE_H_