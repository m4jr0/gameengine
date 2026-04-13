#version 460 core

// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

const uint InvalidSkinningMatrixOffset = 0xFFFFFFFFu;
const uint InvalidJointIndex = 0xFFFFu;

struct ProxyLocalData {
  vec4 localCenter;
  vec4 localMaxExtents;
  mat4 transform;
  uint skinningOffset;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormals;
layout(location = 2) in vec4 inTangents;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec4 inColor;
layout(location = 5) in uvec4 inJointIndices;
layout(location = 6) in vec4 inJointWeights;

layout(std140, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  vec4 ambientColor;
  vec4 viewPos;
}
globalUbo;

layout(location = 1) out FragmentData {
  vec4 ambientColor;
  vec2 texCoord;
  vec3 normals;
  vec4 tangents;  // xyz = tangent, w = sign.
  vec3 viewPos;
  vec3 fragPos;
  vec4 color;
}
outData;

layout(std430, binding = 20) readonly buffer InProxyLocalDatasSsbo {
  ProxyLocalData inProxyLocalDatas[];
};

layout(std430, binding = 21) readonly buffer InProxyIdsSsbo {
  uint inProxyIds[];
};

layout(std430, binding = 27) readonly buffer InSkinningMatricesSsbo {
  mat4 inSkinningMatrices[];
};

mat4 ComputeSkinMatrix(uint offset) {
  if (offset == InvalidSkinningMatrixOffset) {
    return mat4(1.0);
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

  return skin;
}

void main() {
  int instanceIndex = gl_BaseInstance + gl_InstanceID;
  uint proxyId = inProxyIds[instanceIndex];
  ProxyLocalData proxy = inProxyLocalDatas[proxyId];

  mat4 model = proxy.transform;
  mat4 skin = ComputeSkinMatrix(proxy.skinningOffset);
  mat4 objectToWorld = model * skin;

  vec4 worldPos4 = objectToWorld * vec4(inPosition, 1.0);
  vec3 worldPos = worldPos4.xyz;

  outData.texCoord = inTexCoord;
  outData.color = inColor;
  outData.fragPos = worldPos;

  mat3 normalMatrix = transpose(inverse(mat3(objectToWorld)));
  vec3 worldNormal = normalize(normalMatrix * inNormals);
  outData.normals = worldNormal;

  vec3 worldTangent = normalize(normalMatrix * inTangents.xyz);
  worldTangent =
      normalize(worldTangent - worldNormal * dot(worldNormal, worldTangent));
  outData.tangents = vec4(worldTangent, inTangents.w);

  outData.ambientColor = globalUbo.ambientColor;
  outData.viewPos = globalUbo.viewPos.xyz;

  gl_Position = globalUbo.projection * globalUbo.view * vec4(worldPos, 1.0);
}