# version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 in_tangent;
layout (location = 3) in vec3 in_bitangent;
layout (location = 4) in vec2 in_texture_coordinates;

out vec2 texture_coordinates;

uniform mat4 mvp;

void main() {
    texture_coordinates = in_texture_coordinates;
    gl_Position = mvp * vec4(in_position, 1);
}
