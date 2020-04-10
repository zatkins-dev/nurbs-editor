#version 450 core

// nurbs.vert: A vertex shader that hands weighted control points off to a tesselation shader and
//             fragment shader that implements the Phong lighting model on a per-fragment basis.
//

// Per-vertex attributes
layout(location = 0) in vec4 mcPosition; // position in model coordinates

// 2. Transformation
uniform mat4 mc_ec;

void main(void) { gl_Position = mc_ec * mcPosition; }
