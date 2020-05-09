#version 330 core

uniform MaterialProps
{
    vec3 albedo;
};
uniform sampler2D tex;

in vec2 uv;
in vec3 normal;
out vec3 colour;

vec3 SUN_DIR = vec3(-0.5774, -0.5774, -0.5774);

void main() {
    colour = albedo * texture(tex, uv).rgb * clamp(dot(normal, -SUN_DIR), 0.1, 1.0);
}