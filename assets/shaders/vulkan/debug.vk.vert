#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormals;
layout(location = 2) in vec3 inTangents;
layout(location = 3) in vec3 inBitangents;
layout(location = 4) in vec2 inTexCoord;
layout(location = 5) in vec4 inColor;

layout(set = 0, binding = 0) uniform global_uniform_object {
    mat4 projection;
    mat4 view;
} global_ubo;

layout(push_constant) uniform push_constants {
    mat4 model;
} constants;

layout(location = 1) out struct out_data_object {
    vec4 color;
} outData;

void main() {
    outData.color = inColor;
    gl_Position = global_ubo.projection * global_ubo.view * constants.model * vec4(inPosition, 1.0);
}
