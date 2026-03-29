#version 330 core

in vec3 vFragPos;
in vec3 vNormal;
in vec2 vTexCoord;

uniform int       uUseTexture;
uniform sampler2D uTexture;

uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform vec3 uViewPos;

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
    vec4 param; // x = intensity, y = radius
};

struct DirLight
{
    vec4 dir;
    vec4 color;
    vec4 param; // x = intensity
};

struct SpotLight
{
    vec4 pos;
    vec4 dir;
    vec4 color;
    vec4 param; // x = intensity, y = innerCutoff, z = outerCutoff
};

layout(std140) uniform PointLightBuffer
{
    vec4 PointCount; // x = count
    PointLight PointLights[MAX_POINT_LIGHTS];
};

layout(std140) uniform DirLightBuffer
{
    vec4 DirCount; // x = count
    DirLight DirLights[MAX_DIR_LIGHTS];
};

layout(std140) uniform SpotLightBuffer
{
    vec4 SpotCount; // x = count
    SpotLight SpotLights[MAX_SPOT_LIGHTS];
};

out vec4 FragColor;



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

	vec3 result = ambient + diffuse + specular;

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
	
	float diff = max(dot(normal, lightDirection), 0.0f);
	vec3 diffuse = diff * light.color.xyz * diffuseTex;
	
    float spec = pow(max(dot(normal, halfwayDirection), 0.0), 64.0);
    vec3 specular = spec * light.color.xyz * specTex;
	
	float dist = length(light.pos.xyz - vFragPos);
	float attenuation = 1.0 / (constant + linear * dist / light.param.y + quadratic * (dist * dist)/ (light.param.y * light.param.y));
	
	ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
	
	vec3 result = ambient + diffuse + specular;
	
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
    float epsilon = light.param.y - light.param.z;
    float intensity = clamp((theta - light.param.z) / epsilon, 0.0, 1.0);
	
	vec3 ambient = 0.3 * light.color.xyz * diffuseTex;
	
	float diff = max(dot(normal, lightDirection), 0.0);
	vec3 diffuse = diff * attenuation * intensity * light.color.xyz * diffuseTex;
	
    float spec = pow(max(dot(normal, halfwayDirection), 0.0), 32);
    vec3 specular = spec * light.color.xyz * attenuation * intensity * specTex;
	
	vec3 result = ambient + diffuse + specular;

	return result * light.param.x;
}


void main() 
{
    // Base color: texture or solid color
    vec3 baseColor = (uUseTexture == 1)
        ? texture(uTexture, vTexCoord).rgb
        : vec3(0.72, 0.45, 0.20);   // warm orange fallback

	vec3 color;

	for (int i = 0; i < PointCount.x; i++)
	{
		color += CalcPointLight(PointLights[i], baseColor, baseColor);
	}
	for (int i = 0; i < DirCount.x; i++)
	{
		color += CalcDirectionalLight(DirLights[i], baseColor, baseColor);
	}
	for (int i = 0; i < SpotCount.x; i++)
	{
		color += CalcSpotLight(SpotLights[i], baseColor, baseColor);
	}

    FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}