#version 460

#ifdef COMET_VALIDATION_DEBUG_PRINTF_EXT
#extension GL_EXT_debug_printf : enable
#endif // COMET_VALIDATION_DEBUG_PRINTF_EXT

layout(location = 0) in vec3 inPosition;

layout(std140, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
}
globalUbo;

void main() {
  gl_Position = globalUbo.projection * globalUbo.view * vec4(inPosition, 1.0);
}
