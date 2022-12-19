#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormals;
layout(location = 2) in vec4 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(set = 0, binding = 0) uniform global_uniform_object {
    mat4 projection;
    mat4 view;
    vec4 ambientColor;
    vec3 viewPos;
} global_ubo;

layout(push_constant) uniform push_constants {
    mat4 model;
} constants;

layout(location = 1) out struct out_data_object {
    vec4 ambientColor;
    vec2 texCoord;
    vec3 normals;
    vec3 viewPos;
    vec3 fragPos;
    vec4 color;
} outData;

void main() {
    outData.texCoord = inTexCoord;
    outData.color = inColor;
    outData.fragPos = vec3(constants.model * vec4(inPosition, 1.0));
    mat3 modelMat3 = mat3(constants.model);
    outData.normals = normalize(modelMat3 * inNormals);
    outData.ambientColor = global_ubo.ambientColor;
    outData.viewPos = global_ubo.viewPos;

    gl_Position = global_ubo.projection * global_ubo.view * constants.model * vec4(inPosition, 1.0);
}
