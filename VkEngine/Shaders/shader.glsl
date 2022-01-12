const int LIGHT_MAX_COUNT = 8;

#ifndef EXT
#define EXT

#define PI 3.1415926538

vec2 RotateRadians(vec2 v, float radians)
{
    float ca = cos(radians);
    float sa = sin(radians);
    return vec2(ca * v.x - sa * v.y, sa * v.x + ca * v.y);
}

vec2 RotateDegrees(vec2 v, float degrees)
{
    return RotateRadians(v, degrees * (PI/180));
}

vec4 ToWorldPos(vec3 worldPos, vec3 vertPos, vec3 camPos, float rotation, float scale, float camRotation, float aspectRatio, float clipFar)
{
    vec3 pos = worldPos - camPos;
    vertPos *= scale;

    // Scale 2d position based on distance.
    float dis = pos.z + vertPos.z;
    vec2 pos2d = pos.xy + RotateDegrees(vertPos.xy, rotation);
    pos2d = RotateDegrees(pos2d, -camRotation);
    pos2d /= dis;
    pos2d.y *= aspectRatio;

    // Add depth.
    vec3 finalPos = vec3(pos2d, dis / clipFar);
    return vec4(finalPos, 1);
}

#endif
