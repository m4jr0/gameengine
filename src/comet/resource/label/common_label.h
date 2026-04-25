// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_LABEL_COMMON_LABEL_H_
#define COMET_COMET_RESOURCE_LABEL_COMMON_LABEL_H_

#include "comet/core/essentials.h"
#include "comet/resource/type/common.h"

namespace comet {
namespace resource {
const schar* GetCompressionModeLabel(CompressionMode mode);
const schar* GetResourceLifeSpanLabel(ResourceLifeSpan life_span);
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_LABEL_COMMON_LABEL_H_