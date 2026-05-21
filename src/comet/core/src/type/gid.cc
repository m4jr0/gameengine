// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/type/gid.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace gid {
Gid GenerateNewGeneration(Gid id) noexcept {
  const auto index{GetIndex(id)};
  const auto generation{(GetGeneration(id) + 1) &
                        ((Gid{1} << kGenerationBits) - 1)};
  return Generate(index, generation);
}
}  // namespace gid
}  // namespace comet