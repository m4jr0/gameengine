// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef EDITOR_EDITOR_HPP_
#define EDITOR_EDITOR_HPP_

#include "comet/comet.h"
#include "comet_precompile.h"

namespace comet {
class CometEditor : public Engine {
 public:
  CometEditor();

  void Initialize() override;

 protected:
  void Exit() override;
#ifdef _WIN32
  static BOOL WINAPI HandleConsole(DWORD);
#endif  // _WIN32
};
}  // namespace comet

#endif  // EDITOR_EDITOR_HPP_
