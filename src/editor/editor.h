// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_EDITOR_H_
#define COMET_EDITOR_EDITOR_H_

#include "comet_precompile.h"

#include "comet/comet.h"

namespace comet {
namespace editor {
class CometEditor : public core::Engine {
 public:
  CometEditor() = default;
  CometEditor(const CometEditor&) = delete;
  CometEditor(CometEditor&&) = delete;
  CometEditor& operator=(const CometEditor&) = delete;
  CometEditor& operator=(CometEditor&&) = delete;
  virtual ~CometEditor() = default;

  void Initialize() override;

 protected:
  void Exit() override;
#ifdef _WIN32
  static BOOL WINAPI HandleConsole(DWORD window_event);
#endif  // _WIN32
};
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_EDITOR_H_
