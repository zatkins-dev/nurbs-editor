#version 450 core

// addNormal.gsh - add normal vectors to triangles as they pass through.

layout(triangles) in;
layout(triangle_strip, max_vertices = 12) out;

in PVA {
    vec3 ecPosition;
    vec3 ecUnitNormal;
    vec2 texCoords;
}
pvaIn[];

out PVA {
    vec3 ecPosition;
    vec3 ecUnitNormal;
    vec2 texCoords;
}
pvaOut;

void main() {
    vec3 v1 = pvaIn[2].ecPosition - pvaIn[1].ecPosition;
    vec3 v2 = pvaIn[0].ecPosition - pvaIn[1].ecPosition;
    vec3 n = normalize(cross(v1, v2));

    for (int i = 0; i < 3; i++) {
        gl_Position = gl_in[i].gl_Position;
        pvaOut.ecPosition = pvaIn[i].ecPosition;
        pvaOut.ecUnitNormal = n;
        EmitVertex();
    }
    EndPrimitive();
}
