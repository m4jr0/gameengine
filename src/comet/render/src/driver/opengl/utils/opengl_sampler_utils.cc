// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_render_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/opengl/utils/opengl_sampler_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/hash/hash.h"

namespace comet {
namespace render {
namespace gl {
SamplerKey GenerateSamplerKey(const SamplerDescr& descr) {
  SamplerKey hash{0};
  hash = HashCombine(hash, static_cast<SamplerKey>(descr.wrap_s));
  hash = HashCombine(hash, static_cast<SamplerKey>(descr.wrap_t));
  hash = HashCombine(hash, static_cast<SamplerKey>(descr.wrap_r));
  hash = HashCombine(hash, static_cast<SamplerKey>(descr.min_filter));
  hash = HashCombine(hash, static_cast<SamplerKey>(descr.mag_filter));
  hash = HashCombine(hash, static_cast<SamplerKey>(descr.compare_mode));
  hash = HashCombine(hash, static_cast<SamplerKey>(descr.compare_func));
  hash = HashCombine(hash,
                     static_cast<SamplerKey>(descr.border_color.x * 1000.0f));
  hash = HashCombine(hash,
                     static_cast<SamplerKey>(descr.border_color.y * 1000.0f));
  hash = HashCombine(hash,
                     static_cast<SamplerKey>(descr.border_color.z * 1000.0f));
  hash = HashCombine(hash,
                     static_cast<SamplerKey>(descr.border_color.w * 1000.0f));
  hash = HashCombine(hash, static_cast<SamplerKey>(descr.use_border_color));
  return hash;
}
}  // namespace gl
}  // namespace render
}  // namespace comet