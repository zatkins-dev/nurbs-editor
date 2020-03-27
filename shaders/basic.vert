#version 410 core

// basic.vsh: A vertex shader that hands information off to a fragment shader
//            that implements the Phong lighting model on a per-fragment basis.

// Per-vertex attributes
layout(location = 0) in vec3 mcPosition; // position in model coordinates
in vec3 mcNormal;                        // normal vector in model coordinates
in vec2 texCoords;
uniform bool applyLDSinVert = true;
// Output to fragment shader:
out PVA {
    vec3 ecPosition;
    vec3 ecUnitNormal;
    vec2 texCoords;
}
pvaOut;

// 2. Transformation
uniform mat4 mc_ec, ec_lds;

void main(void) {
    vec4 p_ecPosition = mc_ec * vec4(mcPosition, 1.0);
    pvaOut.ecPosition = p_ecPosition.xyz / p_ecPosition.w;
    mat3 normalMatrix = transpose(inverse(mat3x3(mc_ec)));
    pvaOut.ecUnitNormal = normalize(normalMatrix * mcNormal);
    pvaOut.texCoords = texCoords;

    // need to compute projection coordinates for given point
    if (applyLDSinVert)
        gl_Position = ec_lds * p_ecPosition;
    else
        gl_Position = p_ecPosition;
}
