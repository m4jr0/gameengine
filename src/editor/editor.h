// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_EDITOR_H_
#define COMET_EDITOR_EDITOR_H_

#include "comet_precompile.h"

#include "comet/comet.h"
#include "comet/entry_point.h"
#include "comet/event/event.h"
#include "editor/asset/asset_manager.h"
#include "editor/camera_handler.h"

namespace comet {
namespace editor {
class CometEditor : public Engine {
 public:
  CometEditor() = default;
  CometEditor(const CometEditor&) = delete;
  CometEditor(CometEditor&&) = delete;
  CometEditor& operator=(const CometEditor&) = delete;
  CometEditor& operator=(CometEditor&&) = delete;
  virtual ~CometEditor() = default;

  void Update(f64& lag) override;
  void PreLoad() override;
  void PostLoad() override;
  void PostUnload() override;

 protected:
#ifdef COMET_WINDOWS
  static BOOL WINAPI HandleConsole(DWORD window_event);
#endif  // COMET_WINDOWS

 private:
  // TODO(m4jr0): Remove temporary code.
  void PostLoadTmpCode();
  void PostUnloadTmpCode();

  asset::AssetManager asset_manager_{};
  std::unique_ptr<CameraHandler> camera_handler_{nullptr};
};
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_EDITOR_H_
