// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_RENDERING_HANDLE_H_
#define COMET_RENDER_RENDERING_HANDLE_H_

#include "comet/core/essentials.h"
#include "comet/core/type/handle.h"

namespace comet {
namespace rendering {
struct TextureTag {};
using TextureHandle = Handle<TextureTag>;

struct ShaderTag {};
using ShaderHandle = Handle<ShaderTag>;

struct ShaderModuleTag {};
using ShaderModuleHandle = Handle<ShaderModuleTag>;

struct MaterialTag {};
using MaterialHandle = Handle<MaterialTag>;

struct RenderPassHandleTag {};
using RenderPassHandle = Handle<RenderPassHandleTag>;

struct PipelineHandleTag {};
using PipelineHandle = Handle<PipelineHandleTag>;

struct PipelineLayoutHandleTag {};
using PipelineLayoutHandle = Handle<PipelineLayoutHandleTag>;

struct SamplerTag {};
using SamplerHandle = Handle<SamplerTag>;

struct LightTag {};
using LightHandle = Handle<LightTag>;

struct CameraTag {};
using CameraHandle = Handle<CameraTag>;
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_RENDERING_HANDLE_H_