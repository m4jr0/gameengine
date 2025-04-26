#version 460 core

layout(location = 0) out vec4 outColor;

layout(std140, binding = 1) uniform LocalUbo {
  vec4 diffuseColor;
  float shininess;
}
local_ubo;

// diffuseMap, specularMap, normalMap.
uniform sampler2D texSamplers[3];

layout(location = 1) in FragmentData {
  vec4 ambientColor;
  vec2 texCoord;
  vec3 normals;
  vec3 viewPos;
  vec3 fragPos;
  vec4 color;
}
inData;

void main() {
  outColor = texture(texSamplers[0], inData.texCoord) * inData.color;
}