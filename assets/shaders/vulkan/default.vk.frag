#version 460

#ifdef COMET_VALIDATION_DEBUG_PRINTF_EXT
#extension GL_EXT_debug_printf : enable
#endif // COMET_VALIDATION_DEBUG_PRINTF_EXT

layout(location = 0) out vec4 outColor;

layout(location = 1) in struct FragmentData {
  vec4 ambientColor;
  vec2 texCoord;
  vec3 normals;
  vec3 viewPos;
  vec3 fragPos;
  vec4 color;
} inFragmentData;

layout(set = 1, binding = 0) uniform LocalUbo {
  vec4 diffuseColor;
  float shininess;
}
localUbo;

// diffuseMap, specularMap, normalMap.
layout(set = 1, binding = 1) uniform sampler2D inTexSamplers[3];

void main() {
  outColor =
      texture(inTexSamplers[0], inFragmentData.texCoord) * inFragmentData.color;
}