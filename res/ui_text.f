
uniform vec4 mask;

uniform sampler2D font_texture;

in vec2 point;
in vec2 uv;
out vec4 colour;

void main() {
    if(mask_point(point, mask)) {
        discard;
    }

    colour = vec4(1, 1, 1, texture(font_texture, uv).r);
}