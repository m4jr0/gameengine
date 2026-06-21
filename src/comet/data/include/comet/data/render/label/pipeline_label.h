// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_DATA_RENDER_LABEL_PIPELINE_LABEL_H_
#define COMET_DATA_RENDER_LABEL_PIPELINE_LABEL_H_

#include "comet/core/essentials.h"
#include "comet/data/render/pipeline.h"

namespace comet {
namespace render {
const schar* GetCullModeLabel(CullMode mode);
const schar* GetCompareOpLabel(CompareOp op);
const schar* GetPrimitiveTopologyLabel(PrimitiveTopology topo);
const schar* GetPipelineBindTypeLabel(PipelineBindType type);
}  // namespace render
}  // namespace comet

#endif  // COMET_DATA_RENDER_LABEL_PIPELINE_LABEL_H_