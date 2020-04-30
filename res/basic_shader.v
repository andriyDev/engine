#version 330 core
layout(location=0) in vec3 vert_position;
layout(location=3) in vec3 vert_normal;

uniform mat4 mvp;

out vec3 normal;

void main() {
    gl_Position = mvp * vec4(vert_position, 1.0);
    normal = (mvp * vec4(vert_normal, 0.0)).xyz;
}
