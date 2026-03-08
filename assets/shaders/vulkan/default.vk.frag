#version 460

#ifdef COMET_VALIDATION_DEBUG_PRINTF_EXT
#extension GL_EXT_debug_printf : enable
#endif // COMET_VALIDATION_DEBUG_PRINTF_EXT

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2DShadow shadowMap;

layout(location = 1) in struct FragmentData {
  vec4 ambientColor;
  vec2 texCoord;
  vec3 normals;
  vec3 viewPos;
  vec3 fragPos;
  vec4 color;
  vec4 lightClipPos;
} inFragmentData;

layout(set = 1, binding = 0) uniform LocalUbo {
  vec4 diffuseColor;
  float shininess;
}
localUbo;

// diffuseMap, specularMap, normalMap.
layout(set = 1, binding = 1) uniform sampler2D inTexSamplers[3];

float computeShadow(vec4 lightClipPos) {
  vec3 proj = lightClipPos.xyz / lightClipPos.w;

  // NDC [-1, 1] -> UV/depth [0, 1].
  proj = proj * 0.5 + 0.5;

  if (proj.z > 1.0 || proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0) {
    return 1.0;
  }

  float bias = 0.0015;
  return texture(shadowMap, vec3(proj.xy, proj.z - bias));
}

void main() {
  vec4 albedo = texture(inTexSamplers[0], inFragmentData.texCoord) * inFragmentData.color;

  float shadow = computeShadow(inFragmentData.lightClipPos);

  vec3 lit = albedo.rgb * mix(0.2, 1.0, shadow);
  outColor = vec4(lit, albedo.a);

  outColor =
      texture(inTexSamplers[0], inFragmentData.texCoord) * inFragmentData.color;
}