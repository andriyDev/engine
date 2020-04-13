#version 330 core

in vec3 normal;
out vec3 colour;

void main() {
    colour = normal * 0.5 + 0.5 + 0.25;
}