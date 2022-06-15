#version 460

struct ObjectData {
    mat4 model;
};

layout(push_constant) uniform MeshPushContants {
    mat4 render_matrix;
    vec4 data;
} consts;

layout(set = 0, binding = 0) uniform CameraData {
    mat4 view;
    mat4 proj;
    mat4 view_proj;
} cameraData;

layout(set = 0, binding = 1) uniform SceneData {
    vec4 fogColor;
    vec4 fogDistances;
    vec4 ambientColor;
    vec4 sunlightDirection;
    vec4 sunlightColor;
} sceneData;

layout(std140, set = 1, binding = 0) readonly buffer ObjectDataBuffer {
    ObjectData objectData[];
} objectDataBuffer;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inNormals;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec2 fragNormals;

void main() {
    mat4 model = objectDataBuffer.objectData[gl_BaseInstance].model;
    mat4 transform = (cameraData.view_proj * model);
    gl_Position = transform * vec4(inPosition, 1.0f);

    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragNormals = inNormals;
}
