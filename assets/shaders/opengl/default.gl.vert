#version 460

const uint InvalidSkinningMatrixOffset = 0xFFFFFFFFu;
const uint InvalidJointIndex = 0xFFFFu;

struct ProxyLocalData {
  vec3 localCenter;
  vec3 localMaxExtents;
  mat4 transform;
  uint skinningOffset;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormals;
layout(location = 2) in vec3 inTangents;
layout(location = 3) in vec3 inBitangents;
layout(location = 4) in vec2 inTexCoord;
layout(location = 5) in vec4 inColor;
layout(location = 6) in uvec4 inJointIndices;
layout(location = 7) in vec4 inJointWeights;

layout(std140, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  vec4 ambientColor;
  vec3 viewPos;
}
globalUbo;

layout(location = 1) out FragmentData {
  vec4 ambientColor;
  vec2 texCoord;
  vec3 normals;
  vec3 viewPos;
  vec3 fragPos;
  vec4 color;
}
outData;

layout(std430, binding = 10) readonly buffer InProxyLocalDatasSsbo {
  ProxyLocalData inProxyLocalDatas[];
};

layout(binding = 11) readonly buffer InProxyIdsSsbo { uint inProxyIds[]; };

layout(binding = 17) readonly buffer InSkinningMatricesSsbo {
  mat4 inSkinningMatrices[];
};

vec4 ApplySkinning(vec4 pos, uint offset) {
  if (offset == InvalidSkinningMatrixOffset) {
    return pos; // Case: static mesh.
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
  int instanceIndex = gl_BaseInstance + gl_InstanceID;
  uint proxyId = inProxyIds[instanceIndex];
  ProxyLocalData proxy = inProxyLocalDatas[proxyId];
  mat4 model = proxy.transform;

  vec4 skinnedPosition =
      ApplySkinning(vec4(inPosition, 1.0), proxy.skinningOffset);
  vec3 worldPos = vec3(model * skinnedPosition);

  outData.texCoord = inTexCoord;
  outData.color = inColor;
  outData.fragPos = worldPos;

  mat3 modelMat3 = mat3(model);

  outData.normals = normalize(modelMat3 * inNormals);
  outData.ambientColor = globalUbo.ambientColor;
  outData.viewPos = globalUbo.viewPos;

  gl_Position = globalUbo.projection * globalUbo.view * vec4(worldPos, 1.0);
}
