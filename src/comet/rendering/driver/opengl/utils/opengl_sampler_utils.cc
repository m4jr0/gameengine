// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_sampler_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/hash.h"

namespace comet {
namespace rendering {
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
}  // namespace rendering
}  // namespace comet