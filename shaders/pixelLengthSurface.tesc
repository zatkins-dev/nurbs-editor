#version 450 core

layout(vertices = 25) out;

layout(std430, binding = 1) buffer CtrPts { vec4 P[]; };

uniform ivec2 vpWidthHeight = ivec2(500, 500);
uniform float maxPixelLength = 15.0;

uniform mat4 mc_ec, ec_lds;

bool validLDSCoord(float v) {
    if (isinf(v) || isnan(v))
        return false;
    return true;
}

float getPixelMeasure() {
    float xmin = 1.0;
    float xmax = 0.0;
    float ymin = 1.0;
    float ymax = 0.0;
    for (int i = 0; i < P.length(); i++) {
        vec4 pos = ec_lds * mc_ec * P[i];
        vec2 p = pos.xy / pos.w;
        if (validLDSCoord(p.x)) {
            if (xmin > xmax)
                xmin = xmax = p.x;
            else if (p.x < xmin)
                xmin = p.x;
            else if (p.x > xmax)
                xmax = p.x;
        }
        if (validLDSCoord(p.y)) {
            if (ymin > ymax)
                ymin = ymax = p.y;
            else if (p.y < ymin)
                ymin = p.y;
            else if (p.y > ymax)
                ymax = p.y;
        }
    }
    const float minVal = 25.0;
    float fractionX = 0.5 * (xmax - xmin);
    float valX = (fractionX > 0.0) ? fractionX * vpWidthHeight[0] : minVal;
    float fractionY = 0.5 * (ymax - ymin);
    float valY = (fractionY > 0.0) ? fractionY * vpWidthHeight[1] : minVal;
    return max(valX, valY);
}

void main() {
    // if (gl_InvocationID == 0) {
    //     for (int i = 0; i < P.length(); i++)
    //         P[i] = mc_ec * P[i];
    // }
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    float pixelMeasure = getPixelMeasure();

    float outerTessLevel = max(pixelMeasure / maxPixelLength, 1.0);
    float innerTessLevel = max(outerTessLevel - 1.0, 1.0);

    gl_TessLevelOuter[0] = gl_TessLevelOuter[2] = outerTessLevel;
    gl_TessLevelOuter[1] = gl_TessLevelOuter[3] = outerTessLevel;
    gl_TessLevelInner[0] = innerTessLevel;
    gl_TessLevelInner[1] = innerTessLevel;
}
