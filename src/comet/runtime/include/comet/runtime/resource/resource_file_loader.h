// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_RESOURCE_RESOURCE_FILE_LOADER_H_
#define COMET_RUNTIME_RESOURCE_RESOURCE_FILE_LOADER_H_

#include "comet/core/essentials.h"
#include "comet/core/string/tstring.h"
#include "comet/data/resource/resource_file.h"

namespace comet {
namespace resource {
bool LoadResourceFile(CTStringView path, ResourceFile& file);
}  // namespace resource
}  // namespace comet

#endif  // COMET_RUNTIME_RESOURCE_RESOURCE_FILE_LOADER_H_