// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_UI_IMGUI_OPENGL_RENDERER_H_
#define COMET_EDITOR_UI_IMGUI_OPENGL_RENDERER_H_

#include "comet/comet.h"
#include "comet_precompile.h"

namespace comet {
namespace editor {
namespace ui {
// Everything will change when the rendering will be abstracted.
class ImguiOpenGlRenderer : public comet::rendering::RenderingManager {
  virtual void Initialize() override;
  virtual void Update(double, game_object::GameObjectManager&) override;
  virtual void Destroy() override;
  // virtual void Update(double, game_object::GameObjectManager&);
};
}  // namespace ui
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_UI_IMGUI_OPENGL_RENDERER_H_
