// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/resource/shader/shader_resource_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/memory_utils.h"
#include "comet/core/container/array.h"
#include "comet/core/string/tstring.h"
#include "comet/data/render/pipeline.h"
#include "comet/data/render/shader.h"
#include "comet/data/render/utils/shader_utils.h"

namespace comet {
namespace resource {
namespace internal {
constexpr usize kU32Size{sizeof(u32)};
constexpr usize kUsizeSize{sizeof(usize)};
constexpr usize kBoolSize{sizeof(bool)};
constexpr usize kCullModeSize{sizeof(render::CullMode)};
constexpr usize kCompareOpSize{sizeof(render::CompareOp)};
constexpr usize kPrimitiveTopologySize{sizeof(render::PrimitiveTopology)};
constexpr usize kShaderVertexLayoutSize{sizeof(render::ShaderVertexLayout)};
constexpr usize kShaderBindingTypeSize{sizeof(render::ShaderBindingType)};
constexpr usize kShaderBindingScopeSize{sizeof(render::ShaderBindingScope)};
constexpr usize kShaderMemoryLayoutSize{sizeof(render::ShaderMemoryLayout)};
constexpr usize kShaderStageFlagsSize{sizeof(render::ShaderStageFlags)};
constexpr usize kShaderVariableTypeSize{sizeof(render::ShaderVariableType)};
constexpr usize kShaderImageBindingSemanticSize{
    sizeof(render::ShaderImageBindingSemantic)};

usize GetTStringByteSize(const TString& str) {
  return str.GetLengthWithNullTerminator() * sizeof(tchar);
}
}  // namespace internal

ShaderResourceHandler::ShaderResourceHandler(const ResourceHandlerDescr& descr)
    : Base{descr} {}

ResourceFile ShaderResourceHandler::Pack(const ShaderResource& resource,
                                         CompressionMode compression_mode) {
  return PackShaderResource(resource, compression_mode, byte_allocator_);
}

void ShaderResourceHandler::Unpack(const ResourceFile& file,
    ResourceLifeSpan life_span, ShaderResource* resource) {
  UnpackShaderResource(file, byte_allocator_, resource);
  resource->life_span = life_span;
}
}  // namespace resource
}  // namespace comet