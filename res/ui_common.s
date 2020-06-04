
#version 330 core

vec2 transform_rect(vec2 point, vec4 rect)
{
    point *= rect.zw - rect.xy;
    point += rect.xy;
    return point;
}

vec4 normalize_point(vec2 point)
{
    point.y = 1.0 - point.y;
    return vec4(point * 2.0 - 1.0, 0.0, 1.0);
}

bool mask_point(vec2 point, vec4 mask)
{
    return point.x < mask.x || point.y < mask.y || point.x > mask.z || point.y > mask.w;
}
