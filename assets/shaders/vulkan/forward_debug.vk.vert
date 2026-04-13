#version 460 core

// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifdef COMET_VALIDATION_DEBUG_PRINTF_EXT
#extension GL_EXT_debug_printf : enable
#endif  // COMET_VALIDATION_DEBUG_PRINTF_EXT

layout(location = 0) in vec4 inPosition;

layout(set = 0, binding = 0) uniform DebugGlobals {
  mat4 projection;
  mat4 view;
}
globalUbo;

void main() {
  gl_Position = globalUbo.projection * globalUbo.view * inPosition;
}