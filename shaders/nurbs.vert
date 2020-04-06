#version 450 core

// nurbs.vert: A vertex shader that hands weighted control points off to a tesselation shader and
//             fragment shader that implements the Phong lighting model on a per-fragment basis.
//

// Per-vertex attributes
layout(location = 0) in vec4 mcPosition; // position in model coordinates

// 2. Transformation
uniform mat4 mc_ec;

// out PVA {
//     vec3 ecPosition;
//     vec3 ecUnitNormal;
//     vec2 texCoords;
// }
// pvaOut;

void main(void) {
    // pvaOut.ecPosition = mcPosition.xyz / mcPosition.w;
    // pvaOut.ecUnitNormal = vec3(0, 0, 1);
    // pvaOut.texCoords = vec2(0, 0);
    gl_Position = mc_ec * mcPosition;
}
