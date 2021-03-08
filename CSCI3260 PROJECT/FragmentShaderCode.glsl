#version 430


out vec3 outColor;

in vec2 UV; 
in vec3 normalWorld;
in vec3 vertexPositionWorld;

uniform vec3 cameraPosition;
uniform vec3 ambientLight; 

uniform vec3 materialambient;
uniform vec3 materialdiffuse;
uniform vec3 materialspecular;
uniform float materialshininess;

uniform vec3 dirLightdirection; 
uniform vec3 dirLightBrightness;

uniform vec3 pointLightposition; 
uniform float pointLightconstant;
uniform float pointLightlinear;
uniform float pointLightquadratic;
uniform vec3 pointLightcolor;

uniform vec3 spotLightposition; 
uniform vec3 spotLightdirection;
uniform float spotLightcutoff;
uniform float spotLightouterCutOff;
uniform float spotLightconstant;
uniform float spotLightlinear;
uniform float spotLightquadratic;
uniform vec3 spotLightcolor;

uniform sampler2D textureSampler; 


vec3 addDirLight(vec3 lightdirection,vec3 lightintensity, vec3 norm, vec3 viewDir)
{
	vec3 lightDir = normalize(-lightdirection);
	
	float diff = max(dot(norm, lightDir), 0.0);

	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialshininess);

	vec3 diffuse = lightintensity * diff * materialdiffuse;
	vec3 specular = lightintensity * spec * materialspecular;

	return (diffuse + specular);
}

vec3 addPointLight(vec3 lightposition, float lightconstant, float lightlinear, float lightquadratic, vec3 lightcolor, vec3 norm, vec3 vertexPositionWorld, vec3 viewDir)
{
	vec3 lightDir = normalize(lightposition - vertexPositionWorld);

	float diff = max(dot(norm, lightDir), 0.0);

	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialshininess);
	
	// attenuation
	float distance = length(lightposition - vertexPositionWorld);
	float attenuation = 1.0 / (lightconstant + lightlinear * distance + lightquadratic * (distance * distance));

	vec3 diffuse = lightcolor * (diff * materialdiffuse) * attenuation;
	vec3 specular = lightcolor * (spec * materialspecular) * attenuation;

	return (diffuse + specular);
}

vec3 addSpotLight(vec3 lightposition, vec3 lightdirection, float lightcutoff, float lightouterCutOff, float lightconstant, float lightlinear, float lightquadratic, vec3 lightcolor, vec3 norm, vec3 vertexPositionWorld, vec3 viewDir)
{
	vec3 lightDir = normalize(lightposition - vertexPositionWorld);
	
	float diff = max(dot(norm, lightDir), 0.0);
    
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialshininess);
	
    // attenuation
    float distance = length(lightposition - vertexPositionWorld);
    float attenuation = 1.0 / (lightconstant + lightlinear * distance + lightquadratic * (distance * distance));    
    
	// spotlight intensity
    float theta = dot(lightDir, normalize(-lightdirection)); 
    float epsilon = lightcutoff - lightouterCutOff;
    float intensity = clamp((theta - lightouterCutOff) / epsilon, 0.0, 1.0);
	
	vec3 diffuse = lightcolor * (diff * materialdiffuse) * attenuation * intensity;
	vec3 specular = lightcolor * (spec * materialspecular) * attenuation * intensity;
	
	return (diffuse + specular);
}

void main()
{

	vec3 texture = texture(textureSampler, UV).rgb;

	vec3 norm = normalize(normalWorld);
	vec3 viewDir = normalize(cameraPosition - vertexPositionWorld);

	vec3 result = ambientLight * materialambient;
	
	//Add Directional Light
	result += addDirLight(dirLightdirection, dirLightBrightness, norm, viewDir);
	
	//Add Point Light
	result += addPointLight(pointLightposition, pointLightconstant, pointLightlinear, pointLightquadratic, pointLightcolor, norm, vertexPositionWorld, viewDir);
	
	//Add Spot Light
	result += addSpotLight(spotLightposition, spotLightdirection, spotLightcutoff, spotLightouterCutOff, spotLightconstant, spotLightlinear, spotLightquadratic, spotLightcolor, norm, vertexPositionWorld, viewDir);

	//Mix with texture to get final color output
	outColor = result * texture;
}
