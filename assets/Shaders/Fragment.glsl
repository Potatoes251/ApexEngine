#version 330 core

//input
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

//output
out vec4 FragColor;


//texture
uniform sampler2D u_Texture;

//base value
float constant = 1.0;
float linear = 0.09;
float quadratic = 0.032;

//cam position -> eyes
uniform vec3 viewPosition;

//lights structures
struct DirectionalLight {
    vec4 diffuseColor;
	vec4 ambientColor;
	vec4 specularColor;
	
	float shininess;
	
	vec3 lightDir;

};

struct PointLight {
    vec4 diffuseColor;
	vec4 ambientColor;
	vec4 specularColor;
	
	float shininess;
	
	vec3 lightPosition;
};

struct SpotLight {
    vec4 diffuseColor;
	vec4 ambientColor;
	vec4 specularColor;
	
	float shininess;

	vec3 lightPosition;
	
	vec3 spotDirection;
	float cutOff;
	float outerCutOff;
};

//lists of all the lights

uniform DirectionalLight dirLights[5];
uniform PointLight pointLights[5];
uniform SpotLight spotLights[5];

uniform int NUM_DIR_LIGHTS;
uniform int NUM_POINT_LIGHTS;
uniform int NUM_SPOT_LIGHTS;


//----------------------------------------------------------------------------------------------------------------------

vec4 CalcDirectionalLight(DirectionalLight light, vec4 texColor)
{
	//normalization
	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(-light.lightDir);
	vec3 viewDirection = normalize(viewPosition - FragPos);
	vec3 halfwayDirection = normalize(lightDirection + viewDirection);
	
	//ambientColor
	vec4 ambient = light.ambientColor * texColor;
	
	//diffuse
	float diff = max(dot(normal, lightDirection), 0.0f);
	vec4 diffuse = diff * light.diffuseColor;
	
	//specular - blinn
    float spec = pow(max(dot(normal, halfwayDirection), 0.0), light.shininess);
    vec4 specular = spec * light.specularColor;
	
	//combination
	vec4 result = ambient + diffuse + specular;
	
	//return
	return result;
}

vec4 CalcPointLight(PointLight light, vec4 texColor)
{
	//normalization
	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(light.lightPosition - FragPos);
	vec3 viewDirection = normalize(viewPosition - FragPos);
	vec3 halfwayDirection = normalize(lightDirection + viewDirection);
	
	//ambientColor
	vec4 ambient = light.ambientColor * texColor;
	
	//diffuse
	float diff = max(dot(normal, lightDirection), 0.0f);
	vec4 diffuse = diff * light.diffuseColor;
	
	//specular - blinn
    float spec = pow(max(dot(normal, halfwayDirection), 0.0), light.shininess);
    vec4 specular = spec * light.specularColor;
	
	//attenuation
	float distance = length(light.lightPosition - FragPos);
	float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));
	
	//apply attenuation
	ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
	
	//combination
	vec4 result = ambient + diffuse + specular;
	
	//return
	return result;
}

vec4 CalcSpotLight(SpotLight light, vec4 texColor)
{
	//normalization
	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(light.lightPosition - FragPos);
	vec3 viewDirection = normalize(viewPosition - FragPos);
	vec3 halfwayDirection = normalize(lightDirection + viewDirection);

	//attenuation
	float distance = length(light.lightPosition - FragPos);
	float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));
	
	//intensity
    float theta = dot(lightDirection, normalize(-light.spotDirection));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	
	//ambientColor
	vec4 ambient = light.ambientColor * texColor;
	
	//diffuse
	float diff = max(dot(normal, lightDirection), 0.0f);
	vec4 diffuse = diff * light.diffuseColor * attenuation * intensity;
	
	//specular - blinn
    float spec = pow(max(dot(normal, halfwayDirection), 0.0), light.shininess);
    vec4 specular = spec * light.specularColor * attenuation * intensity;

	//combination
	vec4 result = ambient + diffuse + specular;
	
	//return
	return result;
}

//----------------------------------------------------------------------------------------------------------------------

void main()
{
	vec4 texColor = texture(u_Texture, TexCoord);
    vec4 result = vec4(0.0);

    for (int i = 0; i < NUM_DIR_LIGHTS; ++i)
        result += CalcDirectionalLight(dirLights[i], texColor);

    for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
        result += CalcPointLight(pointLights[i], texColor);

    for (int i = 0; i < NUM_SPOT_LIGHTS; ++i)
        result += CalcSpotLight(spotLights[i], texColor);


	//prevent alpha bending and overligting
	//vec4 texColor = texture(u_Texture, TexCoord);
    //FragColor = vec4(clamp(texColor.rgb + result.rgb, 0.0, 1.0), texColor.a);
	
	FragColor = vec4(clamp(result.rgb, 0.0, 1.0), texColor.a);
}