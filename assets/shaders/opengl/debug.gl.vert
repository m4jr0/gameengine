#version 460

layout(location = 0) in vec3 inPosition;

layout(std140, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
}
globalUbo;

void main() {
  gl_Position = globalUbo.projection * globalUbo.view * vec4(inPosition, 1.0);
}
