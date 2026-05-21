// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_RESOURCE_RUNTIME_LOADED_RESOURCE_HANDLE_H_
#define COMET_RUNTIME_RESOURCE_RUNTIME_LOADED_RESOURCE_HANDLE_H_

#include "comet/core/essentials.h"
#include "comet/core/type/handle.h"

namespace comet {
namespace resource {
template <typename Tag>
using LoadedResourceHandle = Handle<Tag>;
}  // namespace resource
}  // namespace comet

#endif  // COMET_RUNTIME_RESOURCE_RUNTIME_LOADED_RESOURCE_HANDLE_H_