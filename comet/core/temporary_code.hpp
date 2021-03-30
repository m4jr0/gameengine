// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_TEMPORARY_CODE_HPP_
#define COMET_TEMPORARY_CODE_HPP_

#include "GL/glew.h"
#include "GLFW/glfw3.h"

namespace comet {
void InitializeTmp(GLuint, GLuint);
void UpdateTmp();
void DestroyTmp();
}  // namespace comet

#endif  // COMET_CORE_GAME_OBJECT_COMPONENT_HPP_
