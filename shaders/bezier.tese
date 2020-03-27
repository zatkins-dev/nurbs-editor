#version 410 core

layout(isolines) in;

const int MAX_NUM_CONTROL_POINTS = 4;

uniform int degree = 3;

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

mat4 C = mat4( 
    vec4(0.0, 0.0, 0.0, 0.0), 
    vec4(1.0, 1.0, 0.0, 0.0), 
    vec4(1.0, 2.0, 1.0, 0.0),
    vec4(1.0, 3.0, 3.0, 1.0)
);

float toIntegerPower(float b, int n) {
    float ret = 1.0;
    for (int i = 0; i < n; i++)
        ret *= b;
    return ret;
}

float B(int i, int n, float t) {
    return C[n][i] * toIntegerPower(t, i) * toIntegerPower(1.0 - t, n - i);
}

vec4 P(float t) {
    vec4 sum = vec4(0.0);
    for (int i = 0; i <= degree; i++)
        sum += B(i, degree, t) * gl_in[i].gl_Position;
    return sum;
}

void main() {
    vec4 pos = P(gl_TessCoord[0]);
    pvaOut.ecPosition = pos.xyz / pos.w;
    pvaOut.ecUnitNormal = pvaIn[0].ecUnitNormal;
    pvaOut.texCoords = pvaIn[0].texCoords;
    gl_Position = pos;
}
