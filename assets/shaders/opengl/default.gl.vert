#version 460

struct ProxyLocalData {
  vec3 localCenter;
  vec3 localMaxExtents;
  mat4 transform;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormals;
layout(location = 2) in vec3 inTangents;
layout(location = 3) in vec3 inBitangents;
layout(location = 4) in vec2 inTexCoord;
layout(location = 5) in vec4 inColor;

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

void main() {
  uint proxyId = inProxyIds[gl_InstanceID];
  mat4 model = inProxyLocalDatas[proxyId].transform;

  outData.texCoord = inTexCoord;
  outData.color = inColor;
  outData.fragPos = vec3(model * vec4(inPosition, 1.0));

  mat3 modelMat3 = mat3(model);
  outData.normals = normalize(modelMat3 * inNormals);
  outData.ambientColor = globalUbo.ambientColor;
  outData.viewPos = globalUbo.viewPos;

  gl_Position =
      globalUbo.projection * globalUbo.view * model * vec4(inPosition, 1.0);
}
