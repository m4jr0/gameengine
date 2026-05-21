// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_RESOURCE_SHADER_SHADER_MODULE_RESOURCE_HANDLE_H_
#define COMET_RUNTIME_RESOURCE_SHADER_SHADER_MODULE_RESOURCE_HANDLE_H_

#include "comet/data/resource/shader/shader_module_resource.h"
#include "comet/runtime/resource/loaded_resource_handle.h"

namespace comet {
namespace resource {
using ShaderModuleResourceHandle =
    LoadedResourceHandle<ShaderModuleResourceTag>;
}  // namespace resource
}  // namespace comet

#endif  // COMET_RUNTIME_RESOURCE_SHADER_SHADER_MODULE_RESOURCE_HANDLE_H_