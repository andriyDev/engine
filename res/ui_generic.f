
uniform vec4 colour_tint;

uniform vec4 mask;

in vec2 point;
in vec2 uv;
out vec4 colour;

void main() {
    if(mask_point(point, mask)) {
        discard;
    }

    colour = colour_tint;
}