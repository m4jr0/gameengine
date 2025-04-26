#version 460

#ifdef COMET_VALIDATION_DEBUG_PRINTF_EXT
#extension GL_EXT_debug_printf : enable
#endif // COMET_VALIDATION_DEBUG_PRINTF_EXT

layout(location = 0) out vec4 outColor;

void main() {
  outColor = vec4(0.0, 1.0, 0.0, 1.0); // Green.
}