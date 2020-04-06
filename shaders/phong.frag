#version 410 core
// phong.fsh - an implementation of the Phong model for a fragment shader

// Incoming per-vertex attributes
in PVA {
    vec3 ecPosition;
    vec3 ecUnitNormal;
    vec2 texCoords;
}
pvaIn;

// Output:
out vec4 fragColor;

// For translucency:
uniform int sceneHasTranslucentObjects = 0;
uniform int drawingOpaqueObjects = 1;

// For font rendering, part 1:
uniform int renderingFontString = 0;
uniform sampler2D fontTextureMap;
uniform vec4 fontColor = vec4(0.0, 0.0, 0.0, 1.0);

// Material properties
uniform vec3 ka = vec3(0.7, 0.7, 0.7);
uniform vec3 kd = vec3(0.7, 0.7, 0.7);
uniform vec3 ks = vec3(0.7, 0.7, 0.7);
uniform float m = 1.0;
uniform float alpha = 1.0;

// Texture mapping
uniform int usingTextureMap = 0;
uniform int addSpecularAfterTexture = 1;
// If support multiple textures, all of the following would be arrays
uniform sampler2D textureMap;
uniform vec4 textureColorMultiplier = vec4(1.0, 1.0, 1.0, 1.0);
// "texCoordsOutOfRangeSpec" is negative if not initialized; it is 0 if the
// GLSL "texture" function is not to be used at all when s (t) is out of 0..1;
// and it is a normal GL_TEXTURE_WRAP code otherwise.
// uniform ivec2 texCoordsOutOfRangeSpec = ivec2(-1, -1); // not yet initialized by client

// Lighting environment
const int MAX_NUM_LIGHTS = 8;
uniform int actualNumLights = 0;
// Light source positions assumed to be given in eye coordinates:
uniform vec4 p_ecLightPosition[MAX_NUM_LIGHTS];
uniform vec3 lightStrength[MAX_NUM_LIGHTS]; // (r,g,b) strength
uniform vec3 globalAmbient = vec3(0.1, 0.1, 0.1);
uniform bool lightEnabled[MAX_NUM_LIGHTS] = bool[](true, true, true, true, true, true, true, true);
uniform vec3 c0c1c2[MAX_NUM_LIGHTS] =
    vec3[](vec3(1.0, 0.0, 1.0), vec3(1.0, 0.0, 1.0), vec3(1.0, 0.0, 1.0), vec3(1.0, 0.0, 1.0),
           vec3(1.0, 0.0, 1.0), vec3(1.0, 0.0, 1.0), vec3(1.0, 0.0, 1.0), vec3(1.0, 0.0, 1.0));

// Matrix transformation: EC->LDS
uniform mat4 ec_lds;

float attenuation(int i, float distToLightInEC) {
    // Convert to a distance in LDS using ec_lds:
    float distToLightInLDS = distToLightInEC * ec_lds[0][0];
    return 1.0 / (c0c1c2[i][0] + c0c1c2[i][1] * distToLightInLDS +
                  c0c1c2[i][2] * distToLightInLDS * distToLightInLDS);
}

// Incoming geometry is in eye coordinates
void phong(in vec3 Q, in vec3 nHat, out vec3 ambient, out vec3 diffuse, out vec3 specular) {
    diffuse = vec3(0.0, 0.0, 0.0);
    specular = vec3(0.0, 0.0, 0.0);
    vec3 liHat;

    // Determine vHat, unit vector towards viewer
    vec3 vHat;
    if ((ec_lds[0][3] == 0.0) && (ec_lds[1][3] == 0.0) && (ec_lds[2][3] == 0.0))
        // Affine transformation ==> either PARALLEL or ORTHOGONAL
        vHat = normalize(vec3(-ec_lds[2][0] / ec_lds[0][0], -ec_lds[2][1] / ec_lds[1][1], 1.0));
    else
        vHat = normalize(-Q); // The vector from Q to eye at (0,0,0)
    // Does nHat need to be flipped?
    if (dot(nHat, vHat) < 0.0)
        nHat = -nHat;

    for (int lsi = 0; lsi < actualNumLights; lsi++) {
        if (lightEnabled[lsi]) {
            vec3 liHat;
            float distToLightInEC;
            if (p_ecLightPosition[lsi].w > 0.0) // positional source in scene
            {
                vec3 li = p_ecLightPosition[lsi].xyz / p_ecLightPosition[lsi].w - Q;
                distToLightInEC = length(li); // distance Q to i-th light source
                liHat = li / distToLightInEC;
            } else // directional light
                liHat = normalize(p_ecLightPosition[lsi].xyz);
            float nHatDotLiHat = dot(nHat, liHat);
            if (nHatDotLiHat > 0.0) // Light source on correct side of surface, so:
            {
                vec3 liStrength = lightStrength[lsi];
                if (p_ecLightPosition[lsi].w > 0.0)
                    liStrength *= attenuation(lsi, distToLightInEC);
                diffuse += liStrength * nHatDotLiHat;
                vec3 riHat = 2.0 * nHatDotLiHat * nHat - liHat;
                float riHatDotVHat = dot(riHat, vHat);
                if (riHatDotVHat > 0.0) // Viewer on correct side of normal vector, so:
                    specular += liStrength * pow(riHatDotVHat, m);
            }
        }
    }

    ambient = ka * globalAmbient;
    diffuse *= kd;
    specular *= ks;
}

void main(void) {
    // To support font rendering, part 2:
    if (renderingFontString == 1)
        fragColor = texture(fontTextureMap, pvaIn.texCoords) * fontColor;
    // END: To support font rendering, part 2
    else {
        vec3 ambient, diffuse, specular;
        phong(pvaIn.ecPosition, pvaIn.ecUnitNormal, ambient, diffuse, specular);

        // Since ambient, diffuse, specular separate here, could experiment with
        // texture interactions like: ((ambient + diffuse)*textureColor + specular)
        vec4 pColor;
        if (usingTextureMap == 1) {
            vec4 tColor = textureColorMultiplier * texture(textureMap, pvaIn.texCoords);
            if (addSpecularAfterTexture == 1)
                pColor = vec4(ambient + diffuse, alpha) * tColor + vec4(specular, 0.0);
            else
                pColor = vec4(ambient + diffuse + specular, alpha) * tColor;
        } else
            pColor = vec4(ambient + diffuse + specular, alpha);

        float maxC = max(max(pColor.r, pColor.g), pColor.b);
        if (maxC > 1.0)
            pColor.rgb /= maxC;

        // Translucency?
        if (sceneHasTranslucentObjects == 1) {
            if (drawingOpaqueObjects == 1)
                if (pColor.a < 1.0)
                    discard;
                else
                    fragColor = pColor;
            else if (pColor.a < 1.0)
                fragColor = pColor;
            else
                discard;
        } else
            fragColor = pColor;
    }
}
