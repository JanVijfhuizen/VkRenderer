const int LIGHT_MAX_COUNT = 8;

#ifndef EXT
#define EXT

vec4 ToWorldPos(vec3 worldPos, vec3 vertPos, vec3 camPos, float scale, float aspectRatio, float clipFar)
{
    vec3 pos = worldPos - camPos;
    vertPos *= scale;

    // Scale 2d position based on distance.
    float dis = pos.z + vertPos.z;
    vec2 pos2d = pos.xy + vertPos.xy;
    pos2d += vertPos.xy;
    pos2d /= dis;
    pos2d.y *= aspectRatio;

    // Add depth.
    vec3 finalPos = vec3(pos2d, dis / clipFar);
    return vec4(finalPos, 1);
}

#endif
