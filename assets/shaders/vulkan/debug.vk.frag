#version 460

layout(location = 0) out vec4 outColor;

layout(location = 1) in struct in_data_object {
    vec4 color;
} inData;

void main() {
    outColor = inData.color;
}