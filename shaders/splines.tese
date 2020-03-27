#version 410 core

layout(isolines) in;

uniform int degree = 3;

uniform float a = 0.5;

uniform mat4 ec_lds;

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

mat4 M = mat4(
    vec4(a - 1., 2. - a, -1. - a, a),
    vec4(-2. * (a - 1.), 3. * a - 4, 2., -a),
    vec4(a - 1., 1. - 2. * a, a, 0.),
    vec4(0., 1., 0., 0.)
);

vec4 buildSVec4(float s) {
    vec4 ss = vec4(1., 1., 1., 1.);
    for (int i = degree - 1; i >= 0; i--) {
        ss[i] = s * ss[i + 1];
    }
    return ss;
}

mat4 buildPVec4() {
    mat4 P;
    for (int i = 0; i <= degree; i++) {
        P[i] = vec4(gl_in[i].gl_Position);
    }
    return transpose(P);
}

vec4 computeSpline() {
    mat4 P = buildPVec4();
    vec4 s = buildSVec4(gl_TessCoord[0]);
    return (M * s) * P;
}

void main() {
    vec4 pos = computeSpline();
    pvaOut.ecPosition = pos.xyz / pos.w;
    pvaOut.ecUnitNormal = pvaIn[0].ecUnitNormal;
    pvaOut.texCoords = pvaIn[0].texCoords;
    gl_Position = ec_lds * pos;
}
