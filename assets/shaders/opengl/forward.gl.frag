#version 460 core

// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

struct CameraData {
  mat4 projection;
  mat4 view;
  vec4 viewPos;
};

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2DArrayShadow shadowMaps;

layout(std140, binding = 0) uniform FrameGlobalsSsbo { vec4 ambientColor; }
frameGlobals;

layout(std140, binding = 2) uniform ShadowSettingsUbo {
  vec4 params0;
  ivec4 params1;
}
shadowSettings;

layout(std430, binding = 30) readonly buffer CameraDatasSsbo {
  CameraData cameraDatas[];
};

layout(location = 1) in FragmentData {
  vec2 texCoord;
  vec3 normals;
  vec4 tangents;
  vec3 viewPos;
  vec3 fragPos;
  vec4 color;
}
inFragmentData;

layout(std140, binding = 10) uniform LocalUbo {
  vec4 diffuseColor;
  float shininess;
}
localUbo;

layout(binding = 11) uniform sampler2D inTexSamplers[3];

struct GpuLight {
  vec4 positionType;
  vec4 directionIntensity;
  vec4 colorRange;
  vec4 shadowHeader;
  vec4 spotData;
};

struct GpuShadowData {
  mat4 viewProj;
  vec4 cascadeData;
  vec4 biasData;
};

layout(std430, binding = 28) readonly buffer InLightsSsbo {
  GpuLight inLights[];
};

layout(std430, binding = 29) readonly buffer InShadowsSsbo {
  GpuShadowData inShadows[];
};

layout(std140, binding = 50) uniform WorldPushConstants {
  uint lightCount;
  uint cameraIndex;
  uint debugFlags;
}
constants;

const uint WorldDebugDisableTextures = 0x1u;
const uint WorldDebugDisableLighting = 0x2u;
const uint WorldDebugDisableShadows = 0x4u;
const uint WorldDebugShowNormals = 0x8u;

bool hasFlag(uint flag) { return (constants.debugFlags & flag) != 0u; }

const uint LightTypeDirectional = 1u;
const uint LightTypeSpot = 2u;
const uint LightTypePoint = 3u;

const uint ShadowTypeNone = 0u;
const uint ShadowTypeDirectionalOrtho = 1u;
const uint ShadowTypeSpotPerspective = 2u;
const uint ShadowTypePointCubemap = 3u;

const float kSpecularStrength = 0.2;
const float kTau = 6.28318530718;
const vec3 kDebugGreyboxColor = vec3(0.65);
const float kAlphaCutout = 0.2;

const vec2 kPoissonDisk[16] =
    vec2[](vec2(-0.94201624, -0.39906216), vec2(0.94558609, -0.76890725),
           vec2(-0.09418410, -0.92938870), vec2(0.34495938, 0.29387760),
           vec2(-0.91588581, 0.45771432), vec2(-0.81544232, -0.87912464),
           vec2(-0.38277543, 0.27676845), vec2(0.97484398, 0.75648379),
           vec2(0.44323325, -0.97511554), vec2(0.53742981, -0.47373420),
           vec2(-0.26496911, -0.41893023), vec2(0.79197514, 0.19090188),
           vec2(-0.24188840, 0.99706507), vec2(-0.81409955, 0.91437590),
           vec2(0.19984126, 0.78641367), vec2(0.14383161, -0.14100790));

float getCascadeBlendRatio() { return shadowSettings.params0.x; }
float getPcfRadius() { return shadowSettings.params0.y; }
int getPcfSamples() { return int(shadowSettings.params0.z); }
int getCascadeCount() { return int(shadowSettings.params0.w); }

int getDebugSingleCascade() { return shadowSettings.params1.x; }
bool isDebugCascades() { return shadowSettings.params1.y != 0; }
bool isBlendingDisabled() { return shadowSettings.params1.z != 0; }

uint getLightType(GpuLight light) { return uint(light.positionType.w); }
vec3 getLightPosition(GpuLight light) { return light.positionType.xyz; }

vec3 getLightDirection(GpuLight light) {
  return normalize(light.directionIntensity.xyz);
}

float getLightIntensity(GpuLight light) { return light.directionIntensity.w; }
vec3 getLightColor(GpuLight light) { return light.colorRange.rgb; }
float getLightRange(GpuLight light) { return light.colorRange.w; }
uint getShadowType(GpuLight light) { return uint(light.shadowHeader.x); }
int getShadowIndex(GpuLight light) { return int(light.shadowHeader.y); }
int getShadowEntryCount(GpuLight light) { return int(light.shadowHeader.z); }
float getInnerAngle(GpuLight light) { return light.spotData.x; }
float getOuterAngle(GpuLight light) { return light.spotData.y; }

vec3 getCascadeDebugColor(int localCascadeIndex) {
  if (localCascadeIndex == 0) return vec3(1.0, 0.0, 0.0);
  if (localCascadeIndex == 1) return vec3(0.0, 1.0, 0.0);
  if (localCascadeIndex == 2) return vec3(0.0, 0.0, 1.0);
  if (localCascadeIndex == 3) return vec3(1.0, 1.0, 0.0);
  return vec3(1.0, 0.0, 1.0);
}

int selectDirectionalCascade(int firstShadowIndex, int cascadeCount,
                             float viewDepth) {
  for (int i = 0; i < cascadeCount; ++i) {
    GpuShadowData shadow = inShadows[firstShadowIndex + i];

    if (viewDepth <= shadow.cascadeData.y) {
      return firstShadowIndex + i;
    }
  }

  return firstShadowIndex + max(cascadeCount - 1, 0);
}

float hash(vec2 p) {
  vec3 p3 = fract(vec3(p.xyx) * 0.1031);
  p3 += dot(p3, p3.yzx + 33.33);
  return fract((p3.x + p3.y) * p3.z);
}

mat2 computePoissonRotation(vec3 worldPos) {
  float angle = hash(worldPos.xz) * kTau;
  float s = sin(angle);
  float c = cos(angle);
  return mat2(c, -s, s, c);
}

float sampleShadowCompare(vec2 uv, float layer, float depth) {
  return texture(shadowMaps, vec4(uv, layer, depth));
}

float sampleShadowPcf4(vec2 uv, float layer, float depth, vec2 texelSize,
                       float radius) {
  vec2 r = texelSize * radius;

  float result = 0.0;
  result += sampleShadowCompare(uv + vec2(-0.5, -0.5) * r, layer, depth);
  result += sampleShadowCompare(uv + vec2(0.5, -0.5) * r, layer, depth);
  result += sampleShadowCompare(uv + vec2(-0.5, 0.5) * r, layer, depth);
  result += sampleShadowCompare(uv + vec2(0.5, 0.5) * r, layer, depth);

  return result * 0.25;
}

float sampleShadowEntry(int shadowIndex, vec3 worldPos, vec3 shadowNormal,
                        vec3 lightDir) {
  if (shadowIndex < 0) {
    return 1.0;
  }

  GpuShadowData shadow = inShadows[shadowIndex];
  vec4 lightClipPos = shadow.viewProj * vec4(worldPos, 1.0);

  vec3 proj = lightClipPos.xyz / lightClipPos.w;
  proj.xy = proj.xy * 0.5 + 0.5;

  if (proj.z < 0.0 || proj.z > 1.0 || proj.x < 0.0 || proj.x > 1.0 ||
      proj.y < 0.0 || proj.y > 1.0) {
    return 1.0;
  }

  float nDotL = max(dot(shadowNormal, lightDir), 0.0);
  float bias = max(shadow.biasData.x, shadow.biasData.y * (1.0 - nDotL));

  float layer = shadow.cascadeData.z;
  int pcfSamples = clamp(getPcfSamples(), 0, 16);
  float pcfRadius = max(getPcfRadius(), 0.0);
  float depth = proj.z - bias;

  if (pcfSamples <= 0 || pcfRadius <= 0.0) {
    return texture(shadowMaps, vec4(proj.xy, layer, depth));
  }

  vec2 texelSize = 1.0 / vec2(textureSize(shadowMaps, 0).xy);

  if (pcfSamples <= 4) {
    return sampleShadowPcf4(proj.xy, layer, depth, texelSize, pcfRadius);
  }

  mat2 rot = computePoissonRotation(worldPos);
  float result = 0.0;

  for (int i = 0; i < pcfSamples; ++i) {
    vec2 offset = (rot * kPoissonDisk[i]) * texelSize * pcfRadius;
    result += texture(shadowMaps, vec4(proj.xy + offset, layer, depth));
  }

  return result / float(pcfSamples);
}

float computeDirectionalShadow(GpuLight light, vec3 worldPos, vec3 shadowNormal,
                               vec3 lightDir, float viewDepth,
                               out int outLocalCascadeIndex) {
  outLocalCascadeIndex = -1;

  if (getShadowType(light) != ShadowTypeDirectionalOrtho ||
      getShadowIndex(light) < 0 || getShadowEntryCount(light) <= 0) {
    return 1.0;
  }

  int firstIndex = getShadowIndex(light);
  int cascadeCount = getShadowEntryCount(light);
  int forcedCascade = getDebugSingleCascade();
  int selectedIndex = -1;

  if (forcedCascade >= 0) {
    int localIndex = clamp(forcedCascade, 0, cascadeCount - 1);
    selectedIndex = firstIndex + localIndex;
    outLocalCascadeIndex = localIndex;
  } else {
    selectedIndex =
        selectDirectionalCascade(firstIndex, cascadeCount, viewDepth);
    outLocalCascadeIndex = selectedIndex - firstIndex;
  }

  float currentShadow =
      sampleShadowEntry(selectedIndex, worldPos, shadowNormal, lightDir);

  if (isBlendingDisabled()) {
    return currentShadow;
  }

  int localIndex = selectedIndex - firstIndex;

  if (localIndex >= cascadeCount - 1) {
    return currentShadow;
  }

  GpuShadowData shadow = inShadows[selectedIndex];
  float splitNear = shadow.cascadeData.x;
  float splitFar = shadow.cascadeData.y;
  float splitRange = splitFar - splitNear;
  float blendRange = splitRange * getCascadeBlendRatio();
  float blend = smoothstep(splitFar - blendRange, splitFar, viewDepth);

  float nextShadow =
      sampleShadowEntry(selectedIndex + 1, worldPos, shadowNormal, lightDir);

  return mix(currentShadow, nextShadow, blend);
}

vec3 computeDirectionalLight(GpuLight light, vec3 albedo, vec3 shadingNormal,
                             vec3 viewDir, float shadow) {
  vec3 l = normalize(-getLightDirection(light));
  float nDotL = max(dot(shadingNormal, l), 0.0);

  vec3 diffuse =
      albedo * getLightColor(light) * getLightIntensity(light) * nDotL;

  vec3 h = normalize(l + viewDir);
  float spec =
      pow(max(dot(shadingNormal, h), 0.0), max(localUbo.shininess, 1.0));
  vec3 specularTex = texture(inTexSamplers[1], inFragmentData.texCoord).rgb;
  vec3 specular = getLightColor(light) * getLightIntensity(light) * spec *
                  specularTex * nDotL * kSpecularStrength;

  return (diffuse + specular) * shadow;
}

vec3 computeSpotLight(GpuLight light, vec3 fragPos, vec3 albedo,
                      vec3 shadingNormal, vec3 viewDir, float shadow) {
  vec3 toLight = getLightPosition(light) - fragPos;
  float distance = length(toLight);

  if (distance <= 0.0001) {
    return vec3(0.0);
  }

  vec3 l = toLight / distance;
  vec3 spotDir = normalize(getLightDirection(light));

  float cosTheta = dot(l, spotDir);
  float innerCos = cos(getInnerAngle(light));
  float outerCos = cos(getOuterAngle(light));
  float spotFactor =
      clamp((cosTheta - outerCos) / max(innerCos - outerCos, 0.0001), 0.0, 1.0);

  float range = max(getLightRange(light), 0.0001);
  float attenuation = max(1.0 - distance / range, 0.0);
  attenuation *= attenuation;

  float nDotL = max(dot(shadingNormal, l), 0.0);

  vec3 diffuse =
      albedo * getLightColor(light) * getLightIntensity(light) * nDotL;

  vec3 h = normalize(l + viewDir);
  float spec =
      pow(max(dot(shadingNormal, h), 0.0), max(localUbo.shininess, 1.0));
  vec3 specularTex = texture(inTexSamplers[1], inFragmentData.texCoord).rgb;
  vec3 specular = getLightColor(light) * getLightIntensity(light) * spec *
                  specularTex * nDotL * kSpecularStrength;

  return (diffuse + specular) * attenuation * spotFactor * shadow;
}

vec3 computePointLight(GpuLight light, vec3 fragPos, vec3 albedo,
                       vec3 shadingNormal, vec3 viewDir) {
  vec3 toLight = getLightPosition(light) - fragPos;
  float distance = length(toLight);

  if (distance <= 0.0001) {
    return vec3(0.0);
  }

  vec3 l = toLight / distance;
  float range = max(getLightRange(light), 0.0001);
  float attenuation = max(1.0 - distance / range, 0.0);
  attenuation *= attenuation;

  float nDotL = max(dot(shadingNormal, l), 0.0);

  vec3 diffuse =
      albedo * getLightColor(light) * getLightIntensity(light) * nDotL;

  vec3 h = normalize(l + viewDir);
  float spec =
      pow(max(dot(shadingNormal, h), 0.0), max(localUbo.shininess, 1.0));
  vec3 specularTex = texture(inTexSamplers[1], inFragmentData.texCoord).rgb;
  vec3 specular = getLightColor(light) * getLightIntensity(light) * spec *
                  specularTex * nDotL * kSpecularStrength;

  return (diffuse + specular) * attenuation;
}

void main() {
  vec4 albedo4 = inFragmentData.color;

  if (!hasFlag(WorldDebugDisableTextures)) {
    albedo4 *= texture(inTexSamplers[0], inFragmentData.texCoord);
  } else {
    albedo4 *= vec4(kDebugGreyboxColor, 1.0);
  }

  if (albedo4.a < kAlphaCutout) {
    discard;
  }

  vec3 baseNormal = normalize(inFragmentData.normals);

  if (hasFlag(WorldDebugShowNormals)) {
    vec3 n = baseNormal * 0.5 + 0.5;
    outColor = vec4(n, albedo4.a);
    return;
  }

  vec3 shadingNormal = baseNormal;

  if (!hasFlag(WorldDebugDisableTextures)) {
    vec3 T = normalize(inFragmentData.tangents.xyz);
    T = normalize(T - baseNormal * dot(baseNormal, T));

    float tangentSign = inFragmentData.tangents.w < 0.0 ? -1.0 : 1.0;
    vec3 B = normalize(cross(baseNormal, T)) * tangentSign;

    mat3 TBN = mat3(T, B, baseNormal);

    vec3 tangentNormal = texture(inTexSamplers[2], inFragmentData.texCoord).xyz;
    tangentNormal = tangentNormal * 2.0 - 1.0;

    shadingNormal = normalize(TBN * tangentNormal);
  }

  vec3 shadowNormal = baseNormal;
  vec3 albedo = albedo4.rgb;
  vec3 viewDir = normalize(inFragmentData.viewPos - inFragmentData.fragPos);

  CameraData camera = cameraDatas[constants.cameraIndex];
  float viewDepth = -(camera.view * vec4(inFragmentData.fragPos, 1.0)).z;

  vec3 lit = albedo * frameGlobals.ambientColor.rgb;

  if (hasFlag(WorldDebugDisableLighting)) {
    outColor = vec4(albedo, albedo4.a);
    return;
  }

  for (uint i = 0u; i < constants.lightCount; ++i) {
    GpuLight light = inLights[i];
    uint lightType = getLightType(light);

    if (lightType == LightTypeDirectional) {
      vec3 l = normalize(-getLightDirection(light));

      int localCascadeIndex = -1;
      float shadow = 1.0;

      if (!hasFlag(WorldDebugDisableShadows)) {
        shadow = computeDirectionalShadow(light, inFragmentData.fragPos,
                                          shadowNormal, l, viewDepth,
                                          localCascadeIndex);
      }

      vec3 dirLit = computeDirectionalLight(light, albedo, shadingNormal,
                                            viewDir, shadow);

      if (isDebugCascades() && localCascadeIndex >= 0) {
        dirLit *= getCascadeDebugColor(localCascadeIndex);
      }

      lit += dirLit;
    } else if (lightType == LightTypeSpot) {
      vec3 toLight = getLightPosition(light) - inFragmentData.fragPos;
      float dist = length(toLight);
      vec3 l = dist > 0.0001 ? toLight / dist : vec3(0.0, 0.0, 1.0);

      float shadow = 1.0;

      if (!hasFlag(WorldDebugDisableShadows) &&
          getShadowType(light) == ShadowTypeSpotPerspective &&
          getShadowIndex(light) >= 0 && getShadowEntryCount(light) > 0) {
        shadow = sampleShadowEntry(getShadowIndex(light),
                                   inFragmentData.fragPos, shadowNormal, l);
      }

      lit += computeSpotLight(light, inFragmentData.fragPos, albedo,
                              shadingNormal, viewDir, shadow);
    } else if (lightType == LightTypePoint) {
      lit += computePointLight(light, inFragmentData.fragPos, albedo,
                               shadingNormal, viewDir);
    }
  }

  outColor = vec4(lit, albedo4.a);
}