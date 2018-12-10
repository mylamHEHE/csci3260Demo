#version 430 //GLSL version your computer supports

in vec3 textureCoordinate;

uniform samplerCube SkyBox;


out vec4 daColor;

void main()
{
	daColor = texture(SkyBox,textureCoordinate);
}