// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DEBUGGER_IMGUI_UTILS_H_
#define COMET_COMET_RENDERING_DEBUGGER_IMGUI_UTILS_H_

#include "comet/core/essentials.h"

#ifdef COMET_DEBUG
#ifdef COMET_IMGUI
// External. ///////////////////////////////////////////////////////////////////
#include "imgui.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace rendering {
void AddTableRow(const schar* key, const schar* value);

template <typename T, typename KeyFormatter, typename ValueFormatter>
void AddTableEntries(const T& container, KeyFormatter key_formatter,
                     ValueFormatter value_formatter) {
  for (const auto& entry : container) {
    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::Text("%s", key_formatter(entry));

    ImGui::TableSetColumnIndex(1);
    value_formatter(entry);
  }
}
}  // namespace rendering
}  // namespace comet
#endif  // COMET_IMGUI
#endif  // COMET_DEBUG

#endif  // COMET_COMET_RENDERING_DEBUGGER_IMGUI_UTILS_H_
