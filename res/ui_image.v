
layout(location=0) in vec2 vert_position;

uniform vec2 surface_size;
uniform vec4 rect;
uniform vec4 mask;

out vec2 point;
out vec2 uv;

void main() {
    point = transform_rect(vert_position, rect);
    uv = vert_position;
    gl_Position = normalize_point(point, surface_size);
}
