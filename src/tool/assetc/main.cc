// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "asset_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_string.h"
#include "comet/core/date.h"
#include "comet/core/file_system/file_system.h"
#include "comet/math/math_scalar.h"
#include "tool/asset/asset.h"

namespace comet {
namespace tool {
namespace asset {
int main(int argc, char** argv) {
  ToolAllocator allocator;

  comet::InitializeFileSystem({
      .scratch_allocator = &allocator,
  });

  comet::tool::asset::AssetcConfig config{};
  config.asset_root = COMET_CTSTRING_VIEW("assets");
  config.resource_root = COMET_CTSTRING_VIEW("resources");
  config.allocator = &allocator;
  config.force = false;

  comet::tool::asset::AssetProcessor asset_manager;
  asset_manager.Initialize(config);
  asset_manager.Refresh();
  asset_manager.Shutdown();

  comet::ShutdownFileSystem();

  return 0;
}
}  // namespace asset
}  // namespace tool
}  // namespace comet
