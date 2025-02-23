// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "imgui_utils.h"

#ifdef COMET_DEBUG
#ifdef COMET_IMGUI
namespace comet {
namespace rendering {
void AddTableRow(const schar* key, const schar* value) {
  ImGui::TableNextRow();

  ImGui::TableSetColumnIndex(0);
  ImGui::Text("%s", key);

  ImGui::TableSetColumnIndex(1);
  ImGui::Text("%s", value);
}
}  // namespace rendering
}  // namespace comet
#endif  // COMET_IMGUI
#endif  // COMET_DEBUG
