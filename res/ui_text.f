
uniform vec4 mask;

uniform sampler2D font_texture;
uniform vec4 colour;

in vec2 point;
in vec2 uv;
out vec4 colour_out;

void main() {
    if(mask_point(point, mask)) {
        discard;
    }

    colour_out = vec4(1, 1, 1, texture(font_texture, uv).r) * colour;
}