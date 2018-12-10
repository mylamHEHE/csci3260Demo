#version 430  // GLSL version your computer supports

layout(location=0) in vec3 position;


out vec3 textureCoordinate;
//Transformation Matrix
uniform mat4 modelProjectionMatrix;
uniform mat4 modelSkyBoxMatrix;
uniform mat4 modelStatusMatrix;



void main()
{
	vec4 newPosition = modelProjectionMatrix * modelSkyBoxMatrix *vec4(position, 1.0);
	gl_Position =  newPosition.xyww;
	
	textureCoordinate =  position;
	
	
	
}