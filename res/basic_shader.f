#version 330 core

uniform MaterialProps
{
    vec3 albedo;
};
uniform sampler2D tex;

in vec2 uv;
in vec3 normal;
in vec3 tangent;
in vec3 bitangent;
out vec3 colour;

vec3 SUN_DIR = vec3(-0.8, -1, -0.5774);

void main() {
    colour = albedo/* * texture(tex, uv).xyz*/ * clamp(dot(normal, -normalize(SUN_DIR)), 0.05, 1.0);
}