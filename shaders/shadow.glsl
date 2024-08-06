uniform sampler2DArray shadowMap;
layout (std140, binding = 0) uniform LightSpaceMatrices
{
    mat4 lightSpaceMatrices[3];
};
const int numCascades = 3;
layout (std140, binding = 1) uniform ShadowBlock
{
    uniform vec3 lightDir;
    uniform float farPlane;
    uniform vec4 cascadePlaneDistances[(numCascades + 3) / 4];
};

float ShadowCalculation(vec3 worldPos, vec3 worldNormal, vec3 cameraPos) {
    // select cascade layer
    float depthValue = abs(cameraPos.z);

    int layer = -1;
    for (int i = 0; i < numCascades; ++i) {
        if (depthValue < cascadePlaneDistances[i/4][i%4]) {
            layer = i;
            break;
        }
    }
    if (layer == -1) 
        layer = numCascades;

    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(worldPos, 1.0);
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (currentDepth > 1.0)
        return 0.0;

    // calculate bias (based on depth map resolution and slope)
/*     vec3 normal = normalize(worldNormal);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    const float biasModifier = 0.5f;
    if (layer == numCascades)
        bias *= 1 / (farPlane * biasModifier);
    else
        bias *= 1 / (cascadePlaneDistances[layer/4][layer%4] * biasModifier); */

    float bias = 0.0001;
 
    // PCF
    float shadow = 0.0;
    float shadowDepth = texture(shadowMap, vec3(projCoords.xy, layer)).r;
    //shadow += (currentDepth - bias) > shadowDepth ? 0.7 : 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
            shadow += (currentDepth - bias) > pcfDepth ? 0.7 : 0.0;        
        }    
    }
    shadow /= 9.0;
        
    return shadow;
}
