#version 460 core

// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

struct CameraData {
  mat4 projection;
  mat4 view;
  vec4 viewPos;
};

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;

layout(std430, binding = 30) readonly buffer CameraDatasSsbo {
  CameraData cameraDatas[];
};

layout(std430, binding = 31) readonly buffer InDebugCameraFrustumsSsbo {
  mat4 inDebugCameraFrustums[];
};

layout(std140, binding = 50) uniform Constants {
  uint cameraFrustumVertexOffset;
  uint cascadeFrustumVertexOffset;
  uint lightFrustumVertexOffset;
  uint cameraIndex;
  uint debugDrawFlags;
}
constants;

const uint DebugDrawAabbs = 0x1u;
const uint DebugDrawCameraFrustums = 0x2u;
const uint DebugDrawCascadeFrustums = 0x4u;
const uint DebugDrawLightFrustums = 0x8u;
const uint kLineVertexCount = 24u;

bool hasDebugDrawFlag(uint flag) {
  return (constants.debugDrawFlags & flag) != 0u;
}

bool shouldDrawVertex(uint vertexIndex) {
  uint cameraStart = constants.cameraFrustumVertexOffset;
  uint cascadeStart = constants.cascadeFrustumVertexOffset;
  uint lightStart = constants.lightFrustumVertexOffset;

  if (vertexIndex < cameraStart) {
    return hasDebugDrawFlag(DebugDrawAabbs);
  }

  if (vertexIndex < cascadeStart) {
    return hasDebugDrawFlag(DebugDrawCameraFrustums);
  }

  if (vertexIndex < lightStart) {
    return hasDebugDrawFlag(DebugDrawCascadeFrustums);
  }

  return hasDebugDrawFlag(DebugDrawLightFrustums);
}

layout(location = 0) out vec4 outColor;

void main() {
  if (!shouldDrawVertex(uint(gl_VertexID))) {
    outColor = vec4(0.0);
    gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
    return;
  }

  outColor = inColor;

  CameraData camera = cameraDatas[constants.cameraIndex];
  gl_Position = camera.projection * camera.view * inPosition;
}