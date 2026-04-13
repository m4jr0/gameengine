#version 460 core

// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

layout(location = 0) in vec4 inPosition;

layout(std140, binding = 0) uniform DebugGlobals {
  mat4 projection;
  mat4 view;
}
globalUbo;

void main() {
  gl_Position = globalUbo.projection * globalUbo.view * inPosition;
}