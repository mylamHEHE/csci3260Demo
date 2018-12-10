#version 430 //GLSL version your computer supports

in vec2 UV;
in vec3 normalWorld;
in vec3 vertexPositionWorld;
in vec3 theColor;

uniform sampler2D myTextureSampler;
uniform sampler2D myTextureSampler_n;

uniform vec3 ambientLight;
uniform vec3 ambientLight2;
uniform vec3 lightPositionWorld;
uniform vec3 lightPositionWorld2;
uniform vec3 eyePositionWorld;
uniform bool isEarth;
uniform mat4 modelStatusMatrix;

out vec4 daColor;

void main()
{
	vec3 ambientColor = texture(myTextureSampler, UV).rgb;
	vec3 diffuseColor = texture(myTextureSampler, UV).rgb;
	vec3 specularColor = vec3(1.0, 0.0, 0.0);
	vec3 specularColor2 = vec3(0.0, 0.0, 1.0);
	vec3 N = normalize(normalWorld);
	
	if(isEarth){
		N = texture(myTextureSampler_n, UV).rgb;
		N = normalize(N * 2.0 - 1.0);
	}
	
	vec3 lightVectorWorld = normalize(lightPositionWorld - vertexPositionWorld);
	float brightness = dot(lightVectorWorld,N);
	brightness = clamp(brightness,0.0f,1.0f);
	vec4 diffuseLight= vec4(brightness,brightness,brightness,1.0);
	
	vec3 lightVectorWorld2 = normalize(lightPositionWorld2 - vertexPositionWorld);
	float brightness2 = dot(lightVectorWorld2,N);
	brightness2 = clamp(brightness2,0.0f,1.0f);
	vec4 diffuseLight2= vec4(brightness2,brightness2,brightness2,1.0);
	
	vec3 reflectedLightVectorWorld = reflect(-lightVectorWorld,normalWorld);
	vec3 eyeVectorWorld = normalize(eyePositionWorld - vertexPositionWorld);
	float s = clamp(dot(reflectedLightVectorWorld,eyeVectorWorld),0,1);
	s = pow(s,64);
	vec4 specularLight = vec4(s,0,0,1);
	
	vec3 reflectedLightVectorWorld2 = reflect(-lightVectorWorld2,normalWorld);
	float s2 = clamp(dot(reflectedLightVectorWorld2,eyeVectorWorld),0,1);
	s2 = pow(s2,64);
	vec4 specularLight2 = vec4(s2,0,0,1);
	
	
	daColor = vec4(ambientLight,1.0) * vec4(ambientColor,1.0) + clamp(diffuseLight,0,1)* vec4(diffuseColor,1.0) + vec4(specularColor,1.0) * specularLight;
	daColor = daColor + vec4(ambientLight2,1.0) * vec4(ambientColor,1.0) + clamp(diffuseLight2,0,1)* vec4(diffuseColor,1.0) + vec4(specularColor2,1.0) * specularLight2;
	//daColor = vec4(theColor,1.0) + ambientColor * vec4(ambientLight,1.0) + 2.0f* diffuseColor * clamp(diffuseLight,0,1);
}