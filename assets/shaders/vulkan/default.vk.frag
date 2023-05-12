#version 460

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform local_uniform_object {
    vec4 diffuseColor;
    float shininess;
} local_ubo;

// diffuseMap, specularMap, normalMap.
layout(set = 1, binding = 1) uniform sampler2D texSamplers[3];

layout(location = 1) in struct data_object {
    vec4 ambientColor;
    vec2 texCoord;
    vec3 normals;
    vec3 viewPos;
    vec3 fragPos;
    vec4 color;
} inData;

void main() {
    outColor = texture(texSamplers[0], inData.texCoord) * inData.color;
}