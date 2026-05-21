// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/engine/debug/ui/imgui_utils.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_DEBUG
#ifdef COMET_IMGUI
namespace comet {
namespace debug {
void AddTableRow(const schar* key, const schar* value) {
  ImGui::TableNextRow();

  ImGui::TableSetColumnIndex(0);
  ImGui::Text("%s", key);

  ImGui::TableSetColumnIndex(1);
  ImGui::Text("%s", value);
}
}  // namespace debug
}  // namespace comet
#endif  // COMET_IMGUI
#endif  // COMET_DEBUG
