#version 420 core

#define CASCADE_COUNT 4

in vec3 vFragPos;
in vec3 vNormal;
in vec2 vTexCoord;
in vec4 FragPosLightSpace[10];
in float vViewSpaceDepth;

uniform int       uUseTexture;
uniform vec3      uTint;

layout(binding = 0) uniform sampler2D uTexture;

uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform vec3 uViewPos;
uniform float cascadeSplits[CASCADE_COUNT];

// 5-14 regular shadow maps
layout(binding = 5) uniform sampler2D		shadowMap[10];
// 15-19 cube shadow map
layout(binding = 15) uniform samplerCube	cubeShadowMap[5];

const int MAX_POINT_LIGHTS = 10;
const int MAX_DIR_LIGHTS = 10;
const int MAX_SPOT_LIGHTS = 10;

//base value for point light attenuation
float constant = 1.0;
float linear   = 0.9;
float quadratic= 3.2;

// use only vec4 to avoid padding mismatch between cpu and gpu structs
struct PointLight
{
    vec4 pos;
    vec4 color;
    vec4 param; // x = intensity, y = shadowIdx, z = radius, w = farPlane
};

struct DirLight
{
    vec4 dir;
    vec4 color;
    vec4 param; // x = intensity, y = shadowIdx
};

struct SpotLight
{
    vec4 pos;
    vec4 dir;
    vec4 color;
    vec4 param; // x = intensity, y = shadowIdx, z = innerCutoff, w = outerCutoff
};

layout(std140, binding = 0) uniform PointLightBuffer
{
    vec4 PointCount; // x = count
    PointLight PointLights[MAX_POINT_LIGHTS];
};

layout(std140, binding = 1) uniform DirLightBuffer
{
    vec4 DirCount; // x = count
    DirLight DirLights[MAX_DIR_LIGHTS];
};

layout(std140, binding = 2) uniform SpotLightBuffer
{
    vec4 SpotCount; // x = count
    SpotLight SpotLights[MAX_SPOT_LIGHTS];
};

out vec4 FragColor;

int SelectCascade(int shadowIdx)
{
    for (int c = 0; c < CASCADE_COUNT; c++)
    {
        if (vViewSpaceDepth < cascadeSplits[c])
            return shadowIdx + c;  // shadowIdx is the base slot for this light
    }
    return shadowIdx + CASCADE_COUNT - 1; // fallback to last cascade
}

float ShadowCalculation(DirLight light)
{
	int shadowIdx = int(light.param.y);
	int cascadeIdx = SelectCascade(shadowIdx);
	int localCascade = cascadeIdx - shadowIdx;
    vec3 projCoords = FragPosLightSpace[cascadeIdx].xyz / FragPosLightSpace[cascadeIdx].w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0)
        return 0.0;

    float closestDepth  = texture(shadowMap[cascadeIdx], projCoords.xy).r;
    float currentDepth  = projCoords.z;

	float texelSize = 1.0 / textureSize(shadowMap[cascadeIdx], 0).x;

	// make shadow smoother
	float shadow = 0.0;
	for(int x = -1; x <= 1; ++x)
	{
	    for(int y = -1; y <= 1; ++y)
	    {
	        float pcfDepth = texture(shadowMap[cascadeIdx], projCoords.xy + vec2(x, y) * texelSize).r; 
	        shadow += currentDepth > pcfDepth ? 1.0 : 0.0;
	    }    
	}
	shadow /= 9.0;

    return shadow;
}

float ShadowCalculation(SpotLight light)
{
	int shadowIdx = int(light.param.y);
    vec3 projCoords = FragPosLightSpace[shadowIdx].xyz / FragPosLightSpace[shadowIdx].w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0)
        return 0.0;

    float closestDepth  = texture(shadowMap[shadowIdx], projCoords.xy).r;
    float currentDepth  = projCoords.z;

	float texelSize = 1.0 / textureSize(shadowMap[shadowIdx], 0).x;
	
	// fix acne
	float bias = max(0.005 * (1.0 - dot(vNormal, -light.dir.xyz)), 0.005);

	// make shadow smoother
	float shadow = 0.0;
	for(int x = -1; x <= 1; ++x)
	{
	    for(int y = -1; y <= 1; ++y)
	    {
	        float pcfDepth = texture(shadowMap[shadowIdx], projCoords.xy + vec2(x, y) * texelSize).r; 
	        shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
	    }    
	}
	shadow /= 9.0;

	//shadow = currentDepth > closestDepth ? 1.0 : 0.0;

    return shadow;
}

float ShadowCalculation(PointLight light)
{
	int shadowIdx = int(light.param.y);

    vec3 fragToLight = vFragPos - light.pos.xyz;

    float currentDepth = length(fragToLight);

	float shadow = 0;
	int samples = 20;

	vec3 sampleOffsetDirections[20] = vec3[]
	(vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
	   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
	   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
	   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
	   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1));  


	float diskRadius = (1.0 + currentDepth / light.param.w) * 0.02;
	
	for (int i = 0; i < samples; ++i)
	{
	    vec3 offsetDir = normalize(sampleOffsetDirections[i]);
	    float closestDepth = texture(cubeShadowMap[shadowIdx], fragToLight + offsetDir * diskRadius).r;
	
	    closestDepth *= light.param.w;
	
	    if (currentDepth  > closestDepth)
	        shadow += 1.0;
	}
	shadow /= float(samples);

    return shadow;
} 



vec3 CalcDirectionalLight(DirLight light, vec3 diffuseTex, vec3 specTex)
{
	//normalization
	vec3 normal = normalize(vNormal);
	vec3 lightDirection = normalize(-light.dir.xyz);
	vec3 viewDirection = normalize(uViewPos - vFragPos);
	vec3 halfwayDirection = normalize(lightDirection + viewDirection);
	
	vec3 ambient = 0.3 * light.color.xyz * diffuseTex;
	
	float diff = max(dot(normal, lightDirection), 0.0);
	vec3 diffuse = diff * light.color.xyz * diffuseTex;
	
    float spec = pow(max(dot(normal, halfwayDirection), 0.0), 64.0);
    vec3 specular = spec * light.color.xyz * specTex;

	float shadow = 0;
	if (light.param.y >= 0)
		shadow = ShadowCalculation(light); 

	vec3 result = ambient * (1 - (shadow * 0.5)) + (diffuse + specular) * (1 - shadow);

	return result * light.param.x;
}

vec3 CalcPointLight(PointLight light, vec3 diffuseTex, vec3 specTex)
{
	//normalization
	vec3 normal = normalize(vNormal);
	vec3 lightDirection = normalize(light.pos.xyz - vFragPos);
	vec3 viewDirection = normalize(uViewPos - vFragPos);
	vec3 halfwayDirection = normalize(lightDirection + viewDirection);
	
	vec3 ambient = 0.3 * light.color.xyz * diffuseTex;
	
	float diff = max(dot(normal, lightDirection), 0.0);
	vec3 diffuse = diff * light.color.xyz * diffuseTex;
	
    float spec = pow(max(dot(normal, halfwayDirection), 0.0), 64.0);
    vec3 specular = spec * light.color.xyz * specTex;
	
	float dist = length(light.pos.xyz - vFragPos);
	float attenuation = 1.0 / (constant + linear * dist + quadratic * (dist * dist));

	if (dist > light.param.z)
		return vec3(0.0);

    diffuse *= attenuation;
    specular *= attenuation;
	
	float shadow = 0;
	if (light.param.y >= 0)
		shadow = ShadowCalculation(light); 

	vec3 result = ambient * (1 - (shadow * 0.25)) + (diffuse + specular) * (1 - shadow);

	return result * light.param.x;
}

vec3 CalcSpotLight(SpotLight light, vec3 diffuseTex, vec3 specTex)
{
	//normalization
	vec3 normal = normalize(vNormal);
	vec3 lightDirection = normalize(light.pos.xyz - vFragPos);
	vec3 viewDirection = normalize(uViewPos - vFragPos);
	vec3 halfwayDirection = normalize(lightDirection + viewDirection);

	float dist = length(light.pos.xyz - vFragPos);
	float attenuation = 1.0 / (constant + linear * dist + quadratic * (dist * dist));
	
	//intensity (cutoff)
    float theta = dot(lightDirection, normalize(-light.dir.xyz));
    float epsilon = light.param.z - light.param.w;
    float intensity = clamp((theta - light.param.w) / epsilon, 0.0, 1.0);
	
	if (theta < light.param.w)
		return vec3(0.0);

	vec3 ambient = 0.3 * light.color.xyz * diffuseTex;
	
	float diff = max(dot(normal, lightDirection), 0.0);
	vec3 diffuse = diff * attenuation * intensity * light.color.xyz * diffuseTex;
	
    float spec = pow(max(dot(normal, halfwayDirection), 0.0), 32);
    vec3 specular = spec * light.color.xyz * attenuation * intensity * specTex;
	
	float shadow = 0;
	if (light.param.y >= 0)
		shadow = ShadowCalculation(light); 

	vec3 result = ambient * (1 - (shadow * 0.25)) + (diffuse + specular) * (1 - shadow);

	return result * light.param.x;
}


void main() 
{
    // Base color: texture or solid color
    vec3 baseColor = (uUseTexture == 1)
        ? texture(uTexture, vTexCoord).rgb * uTint
        : vec3(0.75, 0.75, 0.75) * uTint;

	vec3 color = vec3(0.0);
	

	for (int i = 0; i < int(PointCount.x); i++)
	{
		color += CalcPointLight(PointLights[i], baseColor, baseColor);
	}
	for (int i = 0; i < int(DirCount.x); i++)
	{
		color += CalcDirectionalLight(DirLights[i], baseColor, baseColor);
	}
	for (int i = 0; i < int(SpotCount.x); i++)
	{
		color += CalcSpotLight(SpotLights[i], baseColor, baseColor);
	}

    FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}