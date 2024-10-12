uniform sampler2DArray shadowMap;
layout (std140, binding = 1) uniform LightSpaceMatrices
{
    mat4 lightSpaceMatrices[3];
};
const int numCascades = 3;
layout (std140, binding = 2) uniform ShadowBlock {
    uniform vec3 lightDir;
    uniform float farPlane;
    uniform vec4 cascadePlaneDistances[(numCascades + 3) / 4];
    uniform float baseBias;
};

const float shadowMagnitude = .65f;
float ShadowPCF(vec2 uvs, int cascadeIdx, float lsDepth, float bias) {

    float shadow = 0.0;
    float shadowDepth = texture(shadowMap, vec3(uvs, cascadeIdx)).r;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    //shadow = (lsDepth - bias) > shadowDepth ? shadowMagnitude : 0.0;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, vec3(uvs + vec2(x, y) * texelSize, cascadeIdx)).r;
            shadow += (lsDepth - bias) > pcfDepth ? shadowMagnitude : 0.0;
        }    
    }
    shadow /= 9.0;
    return shadow;
}

float ShadowCalculation(vec3 worldPos, vec3 worldNormal, vec3 cameraPos) {
    if (lightDir.y < 0)
        return shadowMagnitude;

    //float dotNL = dot(worldNormal, lightDir);
    //float sinTheta = sqrt(1.0 - dotNL * dotNL);
    //float bias = baseBias * sinTheta;
    float bias = max(baseBias * (1.0 - dot(worldNormal, lightDir)), 0.0001);
    //float bias = 0.0001;

    float vsDepth = abs(cameraPos.z);
    float vsNextPlaneDistance;
    int layer = -1;
    for (int i = 0; i < numCascades; ++i) {
        vsNextPlaneDistance = cascadePlaneDistances[i/4][i%4];
        if (vsDepth < vsNextPlaneDistance) {
            layer = i;
            break;
        }
    }
    if (layer == -1) 
        return 0.0;

    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(worldPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float lsDepth = projCoords.z;
    if (lsDepth > 1.0)
        return 0.0;
    float shadowAmount = ShadowPCF(projCoords.xy, layer, lsDepth, bias);
    if (layer == numCascades - 1)
        return shadowAmount;

    // Blend with next cascade if possible:
    float vsFrustumLength = layer == 0 ?
        vsNextPlaneDistance :
        vsNextPlaneDistance - cascadePlaneDistances[(layer-1)/4][(layer-1)%4];

    float distToEdge = (vsNextPlaneDistance - vsDepth) / vsFrustumLength;
    float blendStart = 0.9;
    if (1 - distToEdge < blendStart)
        return shadowAmount;

    // 1. check if worldPos falls into next cascade
    vec4 lsPosNextCascade = lightSpaceMatrices[layer+1] * vec4(worldPos, 1.0);
    vec3 tsPosNextCascade = lsPosNextCascade.xyz / lsPosNextCascade.w;
    tsPosNextCascade = tsPosNextCascade * 0.5 + 0.5;
    float maxCoord = max(max(abs(tsPosNextCascade.x), abs(tsPosNextCascade.y)), abs(tsPosNextCascade.z));
    if (maxCoord > 1)
        return shadowAmount;
    // 2. if it does, calculate the shadow value in that next cascade
    float shadowAmountNextCascade = ShadowPCF(tsPosNextCascade.xy, layer+1, tsPosNextCascade.z, bias);
    // 3. blend with shadowAmount, according to distToEdge
    float mixAmount = smoothstep(0.0f, 1 - blendStart, distToEdge);
    float totalShadow = mix(shadowAmountNextCascade, shadowAmount, mixAmount);
    return totalShadow;
}
