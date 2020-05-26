#version 330 core
layout(location=0) in vec3 vert_position;
layout(location=2) in vec2 vert_uv;
layout(location=3) in vec3 vert_normal;
layout(location=4) in vec3 vert_tangent;
layout(location=5) in vec3 vert_bitangent;

uniform mat4 modelMatrix;
uniform mat4 mvp;

out vec2 uv;
out vec3 normal;
out vec3 tangent;
out vec3 bitangent;

void main() {
    gl_Position = mvp * vec4(vert_position, 1.0);
    uv = vert_uv;
    normal = normalize((modelMatrix * vec4(vert_normal, 0.0)).xyz);
    tangent = normalize((modelMatrix * vec4(vert_tangent, 0.0)).xyz);
    bitangent = normalize((modelMatrix * vec4(vert_bitangent, 0.0)).xyz);
}
