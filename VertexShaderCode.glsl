#version 430  // GLSL version your computer supports

layout(location=0) in vec3 position;
layout(location=1) in vec2 vertexUV;
layout(location=2) in vec3 normal;
layout(location=3) in vec3 vertexColor;

//Transformation Matrix
uniform mat4 modelProjectionMatrix;
uniform mat4 modelSkyBoxMatrix;
uniform mat4 modelStatusMatrix;
uniform mat4 View;

uniform vec3 eyePositionWorld;

uniform vec3 lightPositionWorld;
out vec2 UV;
out vec3 theColor;
out vec3 normalWorld;
out vec3 vertexPositionWorld;

uniform vec3 ambientLight;

void main()
{
	vec4 newPosition = modelProjectionMatrix *modelStatusMatrix* vec4(position, 1.0);
	gl_Position =  newPosition;
	
	UV = vertexUV;
	
	theColor = vertexColor * ambientLight;

	vec4 normalBuffer =  modelStatusMatrix * vec4(normal,0.0f);
	normalWorld = normalBuffer.xyz;
	
	vertexPositionWorld = newPosition.xyz;
	
}