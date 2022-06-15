// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_EDITOR_H_
#define COMET_EDITOR_EDITOR_H_

#include "comet_precompile.h"

#include "comet/comet.h"
#include "comet/entry_point.h"
#include "editor/asset/asset_manager.h"

namespace comet {
namespace editor {
class CometEditor : public Engine {
 public:
  CometEditor() = default;
  CometEditor(const CometEditor&) = delete;
  CometEditor(CometEditor&&) = delete;
  CometEditor& operator=(const CometEditor&) = delete;
  CometEditor& operator=(CometEditor&&) = delete;
  ~CometEditor() = default;

  void PreLoad() override;
  // TODO(m4jr0): Remove temporary code (PostLoad is used to generate (a)
  // model(s) onto the scene).
  void PostLoad() override;
  void PostUnload() override;

 protected:
#ifdef COMET_WINDOWS
  static BOOL WINAPI HandleConsole(DWORD window_event);
#endif  // COMET_WINDOWS

 private:
  asset::AssetManager asset_manager_{};
};
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_EDITOR_H_
