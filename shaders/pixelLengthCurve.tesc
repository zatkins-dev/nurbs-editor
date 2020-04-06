#version 450 core

layout(vertices = 25) out;

uniform ivec2 vpWidthHeight = ivec2(500, 500);
uniform float maxPixelLength = 15.0;

// in PVA {
//     vec3 ecPosition;
//     vec3 ecUnitNormal;
//     vec2 texCoords;
// }
// pva_in[];

// out PVA {
//     vec3 ecPosition;
//     vec3 ecUnitNormal;
//     vec2 texCoords;
// }
// pva_out[];

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
    for (int i = 0; i < gl_in.length(); i++) {
        vec4 pos = gl_in[i].gl_Position;
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
    // pva_out[gl_InvocationID].ecPosition = pva_in[gl_InvocationID].ecPosition;
    // pva_out[gl_InvocationID].ecUnitNormal = pva_in[gl_InvocationID].ecUnitNormal;
    // pva_out[gl_InvocationID].texCoords = pva_in[gl_InvocationID].texCoords;
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    float pixelMeasure = getPixelMeasure();

    float outerTessLevel = max(pixelMeasure / maxPixelLength, 1.0);

    gl_TessLevelOuter[0] = 1;
    gl_TessLevelOuter[1] = outerTessLevel;
}
