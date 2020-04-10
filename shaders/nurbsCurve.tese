#version 450 core

layout(isolines) in;

layout(std430, binding = 3) buffer Knots { float T[]; };

const int MAX_DEGREE = 5;

uniform int order = 4;
uniform int knotIndex = 3;

uniform mat4 ec_lds;
uniform mat4 mc_ec;

out PVA {
    vec3 ecPosition;
    vec3 ecUnitNormal;
    vec2 texCoords;
}
pvaOut;

vec4 evalCurve(float t) {
    vec4 v[MAX_DEGREE + 1];
    for (int i = 0; i < gl_in.length(); i++) {
        v[i] = gl_in[i].gl_Position;
    }
    int o = order - 1 - knotIndex;
    for (int L = 1; L <= order; L++) {
        for (int i = knotIndex; i > knotIndex + L - order; i--) {
            float num = t - T[i];
            float denom = T[i - L + order] - T[i];
            float alpha = num / denom;
            v[i + o] = alpha * v[i + o] + (1 - alpha) * v[i + o - 1];
        }
    }
    return v[knotIndex + o];
}

void main() {
    // Fix parameter ranges
    float t = gl_TessCoord[0];
    float t_min = T[knotIndex], t_max = T[knotIndex + 1];
    t = t * (t_max - t_min) + t_min;
    // vec4 pos = evaluateCurveAtT(t);
    vec4 pos = evalCurve(t);
    pvaOut.ecPosition = pos.xyz / pos.w;
    pvaOut.ecUnitNormal = vec3(0, 0, 1);
    pvaOut.texCoords = vec2(0, 0);
    gl_Position = ec_lds * pos;
}
