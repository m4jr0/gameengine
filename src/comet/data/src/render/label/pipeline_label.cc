// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_data_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/data/render/label/pipeline_label.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug/debug_label.h"

namespace comet {
namespace render {
const schar* GetCullModeLabel(CullMode mode) {
  switch (mode) {
    case CullMode::Unknown:
      return "unknown";
    case CullMode::None:
      return "none";
    case CullMode::Front:
      return "front";
    case CullMode::Back:
      return "back";
    case CullMode::FrontAndBack:
      return "front_and_back";
    default:
      return kUnknownLabel;
  }
}

const schar* GetCompareOpLabel(CompareOp op) {
  switch (op) {
    case CompareOp::Unknown:
      return "unknown";
    case CompareOp::Never:
      return "never";
    case CompareOp::Less:
      return "less";
    case CompareOp::Equal:
      return "equal";
    case CompareOp::LessOrEqual:
      return "less_or_equal";
    case CompareOp::Greater:
      return "greater";
    case CompareOp::NotEqual:
      return "not_equal";
    case CompareOp::GreaterOrEqual:
      return "greater_or_equal";
    case CompareOp::Always:
      return "always";
    default:
      return kUnknownLabel;
  }
}

const schar* GetPrimitiveTopologyLabel(PrimitiveTopology topo) {
  switch (topo) {
    case PrimitiveTopology::Unknown:
      return "unknown";
    case PrimitiveTopology::Points:
      return "points";
    case PrimitiveTopology::Lines:
      return "lines";
    case PrimitiveTopology::LineStrip:
      return "line_strip";
    case PrimitiveTopology::Triangles:
      return "triangles";
    case PrimitiveTopology::TriangleStrip:
      return "triangle_strip";
    default:
      return kUnknownLabel;
  }
}

const schar* GetPipelineBindTypeLabel(PipelineBindType type) {
  switch (type) {
    case PipelineBindType::Unknown:
      return "unknown";
    case PipelineBindType::Graphics:
      return "graphics";
    case PipelineBindType::Compute:
      return "compute";
    default:
      return kUnknownLabel;
  }
}
}  // namespace render
}  // namespace comet