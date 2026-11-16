// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/resource/shader/shader_module_resource_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/memory_utils.h"
#include "comet/core/container/array.h"

namespace comet {
namespace resource {
ShaderModuleResourceHandler::ShaderModuleResourceHandler(
    const ResourceHandlerDescr& descr)
    : Base{descr} {}

ResourceFile ShaderModuleResourceHandler::Pack(const ShaderModuleResource& resource,
    CompressionMode compression_mode) {
  return PackShaderModuleResource(resource, compression_mode, byte_allocator_);
}

void ShaderModuleResourceHandler::Unpack(const ResourceFile& file,
    ResourceLifeSpan life_span, ShaderModuleResource* resource) {
  UnpackShaderModuleResource(file, byte_allocator_,
      ResolveAllocator(byte_allocator_, life_span), resource);
  resource->life_span = life_span;
}}  // namespace resource
}  // namespace comet
