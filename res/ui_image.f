
uniform vec4 colour_tint;

uniform vec4 mask;
uniform vec4 rect;
uniform vec4 corner_radii;

uniform sampler2D image;

in vec2 point;
in vec2 uv;
out vec4 colour;

void main() {
    if(mask_point(point, mask)) {
        discard;
    }

    vec2 center_point = vec2(rect.x + rect.z, rect.y + rect.w) * 0.5;
    vec2 rect_size = vec2(rect.z - rect.x, rect.w - rect.y);

    vec2 point_offset = point - center_point;
    vec2 offset_dir = max(vec2(0.0, 0.0), sign(point_offset));
    uint quadrant, quadrant_n1, quadrant_n2;
    {
        uvec2 v = uvec2(offset_dir);
        quadrant = v.x + v.y * 2u;
        quadrant_n1 = (1u - v.x) + v.y * 2u;
        quadrant_n2 = v.x + (1u - v.y) * 2u;
    }
    offset_dir = 2.0 * offset_dir - 1.0;
    float corner_radius = max(0, min(
        min(corner_radii[quadrant],
        rect_size.x - corner_radii[quadrant_n1]),
        rect_size.y - corner_radii[quadrant_n2]));
    vec2 corner_point = offset_dir * rect_size * 0.5 + center_point - offset_dir * corner_radius;
    point_offset = point - corner_point;
    uvec2 offset_dir2 = uvec2(max(vec2(0.0, 0.0), sign(point_offset)));
    if(offset_dir2 == uvec2(offset_dir) && distance(corner_point, point) > corner_radius) {
        discard;
    }

    colour = colour_tint * texture(image, uv);
}