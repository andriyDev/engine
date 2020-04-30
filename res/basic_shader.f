#version 330 core

in vec3 normal;
out vec3 colour;

vec3 SUN_DIR = vec3(-0.5774, 0.5774, 0.5774);

void main() {
    vec3 albedo = vec3(0.361, 0.620, 0.322);
    colour = albedo * clamp(dot(normal, -SUN_DIR), 0.0, 1.0);
}