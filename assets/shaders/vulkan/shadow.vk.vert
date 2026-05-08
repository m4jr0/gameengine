#version 460 core

// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifdef COMET_VALIDATION_DEBUG_PRINTF_EXT
#extension GL_EXT_debug_printf : enable
#endif  // COMET_VALIDATION_DEBUG_PRINTF_EXT

const uint InvalidSkinningMatrixOffset = 0xFFFFFFFFu;
const uint InvalidJointIndex = 0xFFFFu;

struct ProxyLocalData {
  vec4 localCenter;
  vec4 localMaxExtents;
  mat4 transform;
  uint skinningOffset;
};

struct ProxyInstance {
  uint proxyId;
  uint batchId;
};

layout(location = 0) in vec3 inPosition;
layout(location = 5) in uvec4 inJointIndices;
layout(location = 6) in vec4 inJointWeights;

layout(push_constant) uniform ShadowPushConstants { mat4 lightViewProj; }
shadowPush;

layout(std430, set = 2, binding = 0) readonly buffer InProxyLocalDatasSsbo {
  ProxyLocalData inProxyLocalDatas[];
};

layout(std430, set = 2, binding = 1) readonly buffer InShadowProxyIdsSsbo {
  uint inShadowProxyIds[];
};

layout(std430, set = 2, binding = 7) readonly buffer InSkinningMatricesSsbo {
  mat4 inSkinningMatrices[];
};

vec4 applySkinning(vec4 pos, uint offset) {
  if (offset == InvalidSkinningMatrixOffset) {
    return pos;
  }

  mat4 skin = mat4(0.0);

  if (inJointWeights.x > 0.0 && inJointIndices.x != InvalidJointIndex) {
    skin += inJointWeights.x * inSkinningMatrices[offset + inJointIndices.x];
  }

  if (inJointWeights.y > 0.0 && inJointIndices.y != InvalidJointIndex) {
    skin += inJointWeights.y * inSkinningMatrices[offset + inJointIndices.y];
  }

  if (inJointWeights.z > 0.0 && inJointIndices.z != InvalidJointIndex) {
    skin += inJointWeights.z * inSkinningMatrices[offset + inJointIndices.z];
  }

  if (inJointWeights.w > 0.0 && inJointIndices.w != InvalidJointIndex) {
    skin += inJointWeights.w * inSkinningMatrices[offset + inJointIndices.w];
  }

  return skin * pos;
}

void main() {
  uint proxyId = inShadowProxyIds[gl_InstanceIndex];
  ProxyLocalData proxy = inProxyLocalDatas[proxyId];

  vec4 skinnedPosition =
      applySkinning(vec4(inPosition, 1.0), proxy.skinningOffset);
  vec4 worldPos = proxy.transform * skinnedPosition;

  gl_Position = shadowPush.lightViewProj * worldPos;
}