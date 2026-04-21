// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_LABEL_RENDERING_PIPELINE_LABEL_H_
#define COMET_COMET_RENDERING_LABEL_RENDERING_PIPELINE_LABEL_H_

#include "comet/core/essentials.h"
#include "comet/rendering/type/rendering_pipeline_type.h"

namespace comet {
namespace rendering {
const schar* GetCullModeLabel(CullMode mode);
const schar* GetCompareOpLabel(CompareOp op);
const schar* GetPrimitiveTopologyLabel(PrimitiveTopology topo);
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_LABEL_RENDERING_PIPELINE_LABEL_H_