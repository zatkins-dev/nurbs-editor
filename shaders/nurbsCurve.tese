#version 450 core

layout(isolines) in;

layout(std430, binding = 3) buffer Knots { float T[]; };

const int MAX_DEGREE = 5;

uniform int order = 4;
uniform int knotIndex = 3;

uniform mat4 ec_lds;
uniform mat4 mc_ec;

// in PVA {
//     vec3 ecPosition;
//     vec3 ecUnitNormal;
//     vec2 texCoords;
// }
// pvaIn[];

out PVA {
    vec3 ecPosition;
    vec3 ecUnitNormal;
    vec2 texCoords;
}
pvaOut;

// void getBasisFcns(float t, inout float fnsAtT[MAX_DEGREE + 1]) {
//     // int K = T.length();
//     // int selectedKnot = -1;
//     // for (int i = 0; i < gl_in.length() + order; i++) {
//     //     int currKnot = order - 1 + i;
//     //     if (T[currKnot] <= t && t < T[currKnot + 1]) {
//     //         selectedKnot = i;
//     //         break;
//     //     }
//     // }
//     // if (selectedKnot == -1)
//     //     selectedKnot = gl_in.length() - 1;

//     int i = knotIndex - (order - 1);
//     for (int j = 0; j < order - 1; j++)
//         fnsAtT[j] = 0;
//     fnsAtT[order - 1] = 1;
//     for (int k = 1; k < order; k++) {
//         int j = order - 1 - k;
//         int ii = j + i;
//         if (abs(T[ii + k + 1] - T[ii + 1]) > 0.000001)
//             fnsAtT[j] = (T[ii + k + 1] - t) / (T[ii + k + 1] - T[ii + 1]) * fnsAtT[j + 1];
//         j++;
//         for (ii = j + i; j < order - 1; j++) {
//             if (abs(T[ii + k] - T[ii]) > 0.000001)
//                 fnsAtT[j] *= (t - T[ii]) / (T[ii + k] - T[ii]);
//             else
//                 fnsAtT[j] = 0;
//             if (abs(T[ii + k + 1] - T[ii + 1]) > 0.000001)
//                 fnsAtT[j] += (T[ii + k + 1] - t) / (T[ii + k + 1] - T[ii + 1]) * fnsAtT[j + 1];
//         }
//         ii = order - 1 + i;
//         if (abs(T[ii + k] - T[ii]) > 0.000001) {
//             fnsAtT[order - 1] *= (t - T[ii]) / (T[ii + k] - T[ii]);
//         }
//     }
// }

// vec4 evaluateCurveAtT(float t) {
//     float fcnsAtT[MAX_DEGREE + 1];
//     getBasisFcns(t, fcnsAtT);
//     vec4 crvPnt = vec4(0);
//     for (int i = 0; i < order; i++)
//         crvPnt += fcnsAtT[i] * gl_in[i].gl_Position;

//     return crvPnt;
// }

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
