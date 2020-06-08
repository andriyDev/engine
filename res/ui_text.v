
layout(location=0) in vec2 vert_position;

uniform vec2 surface_size;
uniform vec4 rect;

#define MAX_CHARACTERS 128

uniform vec4 character_layout[MAX_CHARACTERS * 2];

out vec2 point;
out vec2 uv;

void main() {
    vec4 tex_layout = character_layout[gl_InstanceID * 2];
    vec4 phys_layout = character_layout[gl_InstanceID * 2 + 1];
    point = transform_rect(vert_position, phys_layout + vec4(rect.xy, rect.xy));
    uv = tex_layout.xy + vert_position * tex_layout.zw;
    gl_Position = normalize_point(point, surface_size);
}
