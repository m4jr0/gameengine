#version 460 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormals;
layout(location = 2) in vec4 inColor;
layout(location = 3) in vec2 inTexCoord;

layout (std140, binding = 0) uniform global_uniform_object {
    mat4 projection;
    mat4 view;
    vec4 ambientColor;
    vec3 viewPos;
} global_ubo;

uniform mat4 model;

out struct data_object {
    vec4 ambientColor;
    vec2 texCoord;
    vec3 normals;
    vec3 viewPos;
    vec3 fragPos;
    vec4 color;
} data;

void main() {
    data.texCoord = inTexCoord;
    data.color = inColor;
    data.fragPos = vec3(model * vec4(inPosition, 1.0));
    mat3 modelMat3 = mat3(model);
    data.normals = normalize(modelMat3 * inNormals);
    data.ambientColor = global_ubo.ambientColor;
    data.viewPos = global_ubo.viewPos;

    gl_Position = global_ubo.projection * global_ubo.view * model * vec4(inPosition, 1.0);
}
