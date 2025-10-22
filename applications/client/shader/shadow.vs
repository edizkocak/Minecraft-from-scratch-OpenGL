#version 430
layout(location = 0) in vec3 in_pos;

uniform mat4 model;
uniform mat4 proj_view;

void main() {
    gl_Position = proj_view * model * vec4(in_pos, 1.0f);
}
