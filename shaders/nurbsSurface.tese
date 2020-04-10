#version 450 core

layout(quads) in;

layout(std430, binding = 1) buffer CtrPts { vec4 P[]; };
layout(std430, binding = 2) buffer KnotsS { float S[]; };
layout(std430, binding = 3) buffer KnotsT { float T[]; };
layout(std430, binding = 4) buffer Debug {
    // int st[4][4][2];
    vec4 v[][4];
};

const int MAX_DEGREE = 5;

uniform int orderS = 4;
uniform int nS = 5;

uniform int orderT = 4;
uniform int nT = 5;

uniform mat4 ec_lds;
uniform mat4 mc_ec;

out PVA {
    vec3 ecPosition;
    vec3 ecUnitNormal;
    vec2 texCoords;
}
pvaOut;

int getSpan(in float v, int order, int n, bool isS) {
    int retV;
    for (int ind = orderS - 1; ind < n + 1; ind++) {
        if (isS) {
            if (S[ind] <= v && v < S[ind + 1])
                return ind;
        } else {
            if (T[ind] <= v && v < T[ind + 1])
                return ind;
        }
    }
    return n;
}

int at(int i, int j) { return i * (nT + 1) + j; }

vec4 evalBasisT(float t, int i) {
    int tInd = getSpan(t, orderT, nT, false);

    int oT = orderT - 1 - tInd;

    vec4 vT[MAX_DEGREE + 1];
    for (int j = 0; j < orderT; j++) {
        vT[j] = P[at(i, j - oT)];
        // v[i][j] = vT[j];
    }

    for (int L = 1; L <= orderT; L++) {
        for (int j = tInd; j > tInd + L - orderT; j--) {
            float num = t - T[j];
            float denom = T[j - L + orderT] - T[j];
            float alpha = num / denom;
            if (isnan(alpha) || isinf(alpha))
                alpha = 0;
            vT[j + oT] = alpha * vT[j + oT] + (1 - alpha) * vT[j + oT - 1];
        }
    }

    return vT[tInd + oT];
}

vec4 evalPointOnSurface(float s, float t) {
    int sInd = getSpan(s, orderS, nS, true);

    int oS = orderS - 1 - sInd;

    vec4 vS[MAX_DEGREE + 1];
    for (int i = 0; i < orderS; i++) {
        vS[i] = evalBasisT(t, i - oS);
        // v[4][i] = vS[i];
    }

    for (int L = 1; L <= orderS; L++) {
        for (int i = sInd; i > sInd + L - orderS; i--) {
            float num = s - S[i];
            float denom = S[i - L + orderS] - S[i];
            float alpha = num / denom;
            if (isnan(alpha) || isinf(alpha))
                alpha = 0;
            vS[i + oS] = alpha * vS[i + oS] + (1 - alpha) * vS[i + oS - 1];
        }
    }

    return vS[sInd + oS];
}

void main() {
    // Fix parameter ranges
    float s = gl_TessCoord[0];
    float s_min = S[orderS - 1], s_max = S[nS + 1];
    s = s * (s_max - s_min) + s_min;
    float t = gl_TessCoord[1];
    float t_min = T[orderT - 1], t_max = T[nT + 1];
    t = t * (t_max - t_min) + t_min;
    // vec4 pos = evaluateCurveAtT(t);
    vec4 pos = mc_ec * evalPointOnSurface(s, t);
    pvaOut.texCoords = vec2(s, t);
    pvaOut.ecPosition = pos.xyz / pos.w;
    pvaOut.ecUnitNormal = vec3(0, 0, 1);

    gl_Position = ec_lds * pos;
}
