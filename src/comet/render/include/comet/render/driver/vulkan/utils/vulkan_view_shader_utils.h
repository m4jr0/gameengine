// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_VIEW_SHADER_UTILS_H_
#define COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_VIEW_SHADER_UTILS_H_

#include "comet/core/essentials.h"
#include "comet/runtime/frame/frame_packet.h"
#include "comet/render/driver/vulkan/handler/vulkan_camera_handler.h"
#include "comet/render/driver/vulkan/handler/vulkan_shader_handler.h"
#include "comet/render/driver/vulkan/type/vulkan_shader.h"
#include "comet/render/render_handle.h"
#include "comet/render/type/light.h"

namespace comet {
namespace rendering {
namespace vk {
namespace sharedshaderconsts {
// Set 0.
constexpr u32 kGlobalUboBinding{0};
constexpr u32 kMainShadowMapBinding{1};
constexpr u32 kShadowSettingsBinding{2};

// Set 1.
constexpr u32 kMaterialParamsBinding{0};
constexpr u32 kMaterialTexturesBinding{1};

// Set 2.
constexpr u32 kProxyLocalDatasBinding{0};
constexpr u32 kProxyIdsBinding{1};
constexpr u32 kProxyInstancesBinding{2};
constexpr u32 kIndirectProxiesBinding{3};
constexpr u32 kDebugDataBinding{4};
constexpr u32 kDebugAabbsBinding{5};
constexpr u32 kDebugLineVerticesBinding{6};
constexpr u32 kDebugLightFrustumsBinding{7};
constexpr u32 kMatrixPalettesBinding{7};
constexpr u32 kLightsBinding{8};
constexpr u32 kShadowsBinding{9};
constexpr u32 kCameraDatasBinding{10};
constexpr u32 kDebugCameraFrustumsBinding{11};
constexpr u32 kDebugCascadeFrustumsBinding{12};
}  // namespace sharedshaderconsts

namespace worldshaderconsts {
constexpr ShaderFieldIndex kAmbientColorFieldIndex{0};
constexpr ShaderFieldIndex kShadowSettingsParams0FieldIndex{0};
constexpr ShaderFieldIndex kShadowSettingsParams1FieldIndex{1};
constexpr ShaderPushConstantIndex kLightingPushConstantIndex{0};
}  // namespace worldshaderconsts

namespace worldcullshaderconsts {
constexpr ShaderPushConstantIndex kDrawCountPushConstantIndex{0};
}  // namespace worldcullshaderconsts

namespace worldsparseuploadshaderconsts {
constexpr u32 kSparseUploadWordIndicesBinding{0};
constexpr u32 kSparseUploadSourceWordsBinding{1};
constexpr u32 kSparseUploadDestinationWordsBinding{2};

constexpr ShaderPushConstantIndex kCountPushConstantIndex{0};
}  // namespace worldsparseuploadshaderconsts

namespace debugshaderconsts {
constexpr ShaderFieldIndex kProjectionFieldIndex{0};
constexpr ShaderPushConstantIndex kCountPushConstantIndex{0};
}  // namespace debugshaderconsts

namespace shadowshaderconsts {
constexpr ShaderFieldIndex kShadowLightViewProjFieldIndex{0};
}  // namespace shadowshaderconsts

void AddWorldGlobalFieldUpdates(
    ShaderHandler* shader_handler, ShaderHandle shader_handle,
    const frame::FramePacket* packet,
    frame::FrameArray<ShaderBufferFieldUpdate>& field_updates);

void AddWorldGlobalImageBindings(
    ShaderHandler* shader_handler, const TextureHandler* texture_handler,
    ShaderHandle shader_handle, const TextureMap* shadow_map,
    frame::FrameArray<ShaderImageBindingUpdate>& image_bindings,
    frame::FrameArray<ShaderImageDescriptor>& image_descriptors);

void AddWorldShadowSettingsFieldUpdates(
    ShaderHandler* shader_handler, ShaderHandle shader_handle,
    const ShadowSettings* shadow_settings,
    frame::FrameArray<ShaderBufferFieldUpdate>& field_updates);

void AddCameraBufferBinding(
    ShaderHandler* shader_handler, const CameraHandler* camera_handler,
    ShaderHandle shader_handle, FrameInFlightIndex frame_index,
    frame::FrameArray<ShaderBufferBindingUpdate>& buffer_bindings);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_VIEW_SHADER_UTILS_H_