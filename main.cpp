/*********************************************************
FILE : main.cpp (csci3260 2018-2019 Project)
Skeleton Code: Assignment 2
*********************************************************/
/*********************************************************
Student Information
Student ID:		1155092634
Student Name:	Cheung Kam Ho
Student ID:		1155083016
Student Name:	Lam Ming Yuen
*********************************************************/
/*********************************************************
TO DO List:
Render :two planets,								[Done]
		a spacecraft								[Done]
	    at least three energy rings					[Done]
Object rotation										[Done]
Render :Skybox										[Done]
Render :Asteroid ring								[Done]
rotation of the rocks:								[Done]
Basic Light											[Bugs]
View Point											[Done]
Self-rotation of spacecraft							[Done]
Translation of spacecraft							[Done]
Feedback for passing though an energy ring			[Done]
Normal Mapping										[Done]
*********************************************************/

#define _CRT_SECURE_NO_WARNINGS
#define Window_W 512
#define Window_H 512

#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"
#include "Dependencies\glm\glm.hpp"
#include "Dependencies\glm\gtc\matrix_transform.hpp"
#include "Dependencies\glm\gtc\type_ptr.hpp"
#include <iostream>
#include <fstream>
#include <vector>



using namespace std;
using glm::vec3;
using glm::mat4;

GLint programID;
GLint programID_SkyBox;
// Could define the Vao&Vbo and interaction parameter here
typedef struct obj_t {
	std::vector <glm::vec3> vertices;
	std::vector <glm::vec2> uvs;
	std::vector <glm::vec3> normals;
	GLuint VAO;
	GLuint VBO;
	GLuint texture[2];
}Object;

//========================================================
float Xmove = 1.0f;
float Ymove = 0;
float Zmove = 0;

float SC_x = -10.0f;
float SC_z = 0.0f;

bool passing_ring_1 = false;
bool passing_ring_2 = false;
bool passing_ring_3 = false;

float Rock_Orbit_angle = 0.0f;
//========================================================
Object Skybox;
Object Planet;
	/*
		texture[0] : Earth
		texture[1] : Wonder Star
	*/
GLint normTexture;
bool isEarth = false;
Object spaceCraft;
	/*
		texture[0] : normal
		texture[1] : green light
	*/
Object ring;
	/*
		texture[0] : normal
		texture[1] : green light
	*/
Object rock;
glm::mat4 * Rock_Matrices;

GLuint vertexBuffer;
GLuint uvBuffer;
GLuint normalBuffer;
GLuint TextureID;
GLuint SkyboxID;
GLuint TextureID_n;

GLint doNormalMap;

float rotate_planet = 0;

GLfloat diffuse = 1.0f; 
GLfloat specular = 1.0f;

typedef struct cam_t {
	const GLfloat ZOOM = 45.0f;
	glm::vec3 position = glm::vec3(0.0f + Xmove, 0.0f, 0.0f + Ymove);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 front = glm::vec3((10.0f + Zmove) * cos(0.2f * Xmove), (10.0f + Zmove) * sin(90.0f + 0.2f * Ymove), (10.0f + Zmove) *  sin(0.2f * Xmove));
	glm::mat4 GetViewMatrix(){
		return glm::lookAt(this->position, this->position + this->front, this->up);
	}
}Camera;
static Camera Eye;
//a series utilities for setting shader parameters 
void setMat4(const std::string &name, glm::mat4& value)
{
	unsigned int transformLoc = glGetUniformLocation(programID, name.c_str());
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(value));
}

void setVec4(const std::string &name, glm::vec4 value)
{
	glUniform4fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}
void setVec3(const std::string &name, glm::vec3 value)
{
	glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}
void setFloat(const std::string &name, float value)
{
	glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}
void setInt(const std::string &name, int value)
{
	glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}

bool checkStatus(
	GLuint objectID,
	PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
	PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
	GLenum statusType)
{
	GLint status;
	objectPropertyGetterFunc(objectID, statusType, &status);
	if (status != GL_TRUE)
	{
		GLint infoLogLength;
		objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* buffer = new GLchar[infoLogLength];

		GLsizei bufferSize;
		getInfoLogFunc(objectID, infoLogLength, &bufferSize, buffer);
		cout << buffer << endl;

		delete[] buffer;
		return false;
	}
	return true;
}

bool checkShaderStatus(GLuint shaderID)
{
	return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool checkProgramStatus(GLuint programID)
{
	return checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

string readShaderCode(const char* fileName)
{
	ifstream meInput(fileName);
	if (!meInput.good())
	{
		cout << "File failed to load..." << fileName;
		exit(1);
	}
	return std::string(
		std::istreambuf_iterator<char>(meInput),
		std::istreambuf_iterator<char>()
	);
}

void installShaders()
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	string temp = readShaderCode("VertexShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	temp = readShaderCode("FragmentShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
		return;

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	if (!checkProgramStatus(programID))
		return;

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);
	
	glUseProgram(programID);
}


void installSkyBoxShaders()
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	string temp = readShaderCode("VertexShaderCodeSkybox.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	temp = readShaderCode("FragmentShaderCodeSkybox.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
		return;
	
	programID_SkyBox = glCreateProgram();
	glAttachShader(programID_SkyBox, vertexShaderID);
	glAttachShader(programID_SkyBox, fragmentShaderID);
	glLinkProgram(programID_SkyBox);

	if (!checkProgramStatus(programID_SkyBox))
		return;

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	glUseProgram(programID_SkyBox);
}

void keyboard(unsigned char key, int x, int y)
{
	//TODO: Use keyboard to do interactive events and animation



}

void move(int key, int x, int y)
{
	//TODO: Use arrow keys to do interactive events and animation
	//if (key == GLUT_KEY_RIGHT)
	//	Zmove--;
	//if (key == GLUT_KEY_LEFT)
	//	Zmove++;
	if (key == GLUT_KEY_DOWN) {
		SC_x -= Xmove * glm::cos(Ymove);
		SC_z += Xmove * glm::sin(Ymove);
	}
	if (key == GLUT_KEY_UP) {
		SC_x += Xmove * glm::cos(Ymove);
		SC_z -= Xmove * glm::sin(Ymove);
	}
	if (key == GLUT_KEY_RIGHT) {
		SC_x -= Xmove * glm::cos(Ymove + 90.0f);
		SC_z += Xmove * glm::sin(Ymove + 90.0f);
	}
	if (key == GLUT_KEY_LEFT) {
		SC_x += Xmove * glm::cos(Ymove + 90.0f);
		SC_z -= Xmove * glm::sin(Ymove + 90.0f);
	}

}

void PassiveMouse(int x, int y)
{
	//TODO: Use Mouse to do interactive events and animation
		Ymove = ((float)x - 0.5 *(float)Window_W)/ ((float)Window_W)* -1 * glm::radians(360.0f);
		



}

bool loadOBJ(
	const char * path,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE * file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 6 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

				   // else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}

	return true;
}

GLuint loadBMP_custom(const char * imagepath) {

	printf("Reading image %s\n", imagepath);

	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	unsigned char * data;

	FILE * file = fopen(imagepath, "rb");
	if (!file) { printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); return 0; }

	if (fread(header, 1, 54, file) != 54) {
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (*(int*)&(header[0x1E]) != 0) { printf("Not a correct BMP file\n");    return 0; }
	if (*(int*)&(header[0x1C]) != 24) { printf("Not a correct BMP file\n");    return 0; }

	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);
	if (imageSize == 0)    imageSize = width * height * 3;
	if (dataPos == 0)      dataPos = 54;

	data = new unsigned char[imageSize];
	fread(data, 1, imageSize, file);
	fclose(file);


	GLuint textureID = 0;
	//TODO: Create one OpenGL texture and set the texture parameter 
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;
	glGenerateMipmap(GL_TEXTURE_2D);

	return textureID;
}

void loadBMP_data(const GLchar* facePath, unsigned char* &imageData, unsigned int &width, unsigned int &height) {
	printf("Reading image %s\n", facePath);

	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;

	FILE * file = fopen(facePath, "rb");
	if (!file) { 
		printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", facePath); 
		getchar(); 
	}
	
	if (fread(header, 1, 54, file) != 54) {
		printf("Not a correct BMP file\n");
	}
	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
	}
	if (*(int*)&(header[0x1E]) != 0) { 
		printf("Not a correct BMP file\n"); 
	}
	if (*(int*)&(header[0x1C]) != 24) { 
		printf("Not a correct BMP file\n"); 
	}

	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);
	if (imageSize == 0) {

		imageSize = width * height * 3;
	}
	if (dataPos == 0) {
		dataPos = 54;
	}
	imageData = new unsigned char[imageSize];
	fread(imageData, 1, imageSize, file);
	fclose(file);




}


//Tutorial 9
GLuint loadCubeMap(vector<const GLchar*> faces) {

	unsigned int width;
	unsigned int height;
	unsigned char* image;
	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	
	for (GLuint i = 0; i < faces.size(); i++) {
		
		loadBMP_data(faces[i], image, width, height);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);	
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;

}

void CreateRand_ModelM() {
	int Rock_Num = 250;
	Rock_Matrices = new glm::mat4[Rock_Num];
	srand(glutGet(GLUT_ELAPSED_TIME)); // initialize random seed	
	float radius = 5.0f;
	float offset = 0.5f;
	GLfloat displacement;
	for (unsigned int i = 0; i < Rock_Num; i++)
	{
		glm::mat4 model;
		// 1. translation: displace along circle with 'radius' in range [-offset, offset]
		float angle = (float)i / (float)Rock_Num * 360.0f;
		displacement = (rand() % (int)(2 * offset * 200)) / 100.0f - offset;
		float x = sin(angle) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 200)) / 100.0f - offset;
		float y = displacement * 0.3f; // keep height of field smaller compared to width of x and z
		displacement = (rand() % (int)(2 * offset * 200)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;
		model = glm::translate(model, glm::vec3(x, y, z));

		// 2. scale: Scale between 0.05 and 0.15f
		float scale = (rand() % 10) / 100.0f + 0.05;
		model = glm::scale(model, glm::vec3(scale));

		// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
		float rotAngle = (rand() % 360);
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		// 4. now add to list of matrices
		Rock_Matrices[i] = model;


	}
}

void sendDataToOpenGL()
{
	//TODO:
	//Load objects and bind to VAO & VBO
	//Load texture

	GLfloat skyBoxSize[] =
	{
		
		//right-face
	     -60.0f,     60.0f,	-60.0f,//1
		 -60.0f,	-60.0f,	-60.0f,//2
		  60.0f,	-60.0f,	-60.0f,//3
		  60.0f,	-60.0f,	-60.0f,//4
		  60.0f,	 60.0f, -60.0f,//5
		 -60.0f,	 60.0f,	-60.0f,//6
		//left-face
		 -60.0f,	 60.0f,	 60.0f,//7
		 -60.0f,	-60.0f,	 60.0f,//8
		  60.0f,	-60.0f,	 60.0f,//9
		  60.0f,	-60.0f,	 60.0f,//10
		  60.0f,	 60.0f,  60.0f,//11
		 -60.0f,	 60.0f,	 60.0f,//12
		//bottom-face
		 -60.0f,	-60.0f,	 60.0f,//13
		  60.0f,	-60.0f,	 60.0f,//14
		  60.0f,	-60.0f,	-60.0f,//15
		  60.0f,	-60.0f,	-60.0f,//16
		 -60.0f,	-60.0f,	-60.0f,//17
		 -60.0f,	-60.0f,	 60.0f,//18
		//top-face
		 -60.0f,	 60.0f,	 60.0f,//19
		  60.0f,	 60.0f,	 60.0f,//20
		  60.0f,	 60.0f,	-60.0f,//21
		  60.0f,	 60.0f,	-60.0f,//22
		 -60.0f,	 60.0f,	-60.0f,//23
		 -60.0f,	 60.0f,	 60.0f,//24
		//back-face
		  60.0f,	 60.0f,  60.0f,//25
		  60.0f,	-60.0f,	 60.0f,//26
		  60.0f,	-60.0f,	-60.0f,//27
		  60.0f,	-60.0f,	-60.0f,//28
		  60.0f,	 60.0f, -60.0f,//29
		  60.0f,	 60.0f,  60.0f,//30
		//front-face
		 -60.0f,	 60.0f,  60.0f,//31
		 -60.0f,	-60.0f,	 60.0f,//32
		 -60.0f,	-60.0f,	-60.0f,//33
		 -60.0f,	-60.0f,	-60.0f,//34
		 -60.0f,	 60.0f, -60.0f,//35
		 -60.0f,	 60.0f,  60.0f,//36
		 
	

	};

	glGenVertexArrays(1, &Skybox.VAO);
	glBindVertexArray(Skybox.VAO);
	glGenBuffers(1, &Skybox.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, Skybox.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyBoxSize), &skyBoxSize, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);

	vector<const GLchar*> spaceFaces;
	spaceFaces.push_back("Materials/texture/skybox/purplenebula_rt.bmp");
	spaceFaces.push_back("Materials/texture/skybox/purplenebula_lf.bmp");
	spaceFaces.push_back("Materials/texture/skybox/purplenebula_dn.bmp");
	spaceFaces.push_back("Materials/texture/skybox/purplenebula_up.bmp");
	spaceFaces.push_back("Materials/texture/skybox/purplenebula_bk.bmp");
	spaceFaces.push_back("Materials/texture/skybox/purplenebula_ft.bmp");
	Skybox.texture[0] = loadCubeMap(spaceFaces);
	//printf("%d", Skybox.texture);
	//glBindVertexArray(VAObuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox.texture[0]);
	glUniform1i(TextureID, 0);
//=====================================================
// O Planet
//=====================================================
	
	bool res = loadOBJ("Materials/planet.obj", Planet.vertices, Planet.uvs, Planet.normals);
	glGenVertexArrays(1, &Planet.VAO);
	glBindVertexArray(Planet.VAO);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, Planet.vertices.size() * sizeof(glm::vec3), &Planet.vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glGenBuffers(1, &uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, Planet.uvs.size() * sizeof(glm::vec2), &Planet.uvs[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	
	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, Planet.normals.size() * sizeof(glm::vec2), &Planet.normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	//Earth texture
	Planet.texture[0] = loadBMP_custom("Materials/texture/earthTexture.bmp");
	TextureID = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Planet.texture[0]);
	glUniform1i(TextureID, 0);
	//Normal Mapping
	normTexture = loadBMP_custom("Materials/texture/earth_normal.bmp");

	//Wonder Star texture
	Planet.texture[1] = loadBMP_custom("Materials/texture/WonderStarTexture.bmp");
	TextureID = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, Planet.texture[1]);
	glUniform1i(TextureID, 1);
	

//==========================================================================
// O SpaceCraft
//==========================================================================

	res = loadOBJ("Materials/spaceCraft.obj", spaceCraft.vertices, spaceCraft.uvs, spaceCraft.normals);
	glGenVertexArrays(1, &spaceCraft.VAO);
	glBindVertexArray(spaceCraft.VAO);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, spaceCraft.vertices.size() * sizeof(glm::vec3), &spaceCraft.vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glGenBuffers(1, &uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, spaceCraft.uvs.size() * sizeof(glm::vec2), &spaceCraft.uvs[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, spaceCraft.normals.size() * sizeof(glm::vec2), &spaceCraft.normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	//Normal Mode
	spaceCraft.texture[0] = loadBMP_custom("Materials/texture/spacecraftTexture.bmp");
	TextureID = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, spaceCraft.texture[0]);
	glUniform1i(TextureID, 0);
	//Charging Mode
	spaceCraft.texture[1] = loadBMP_custom("Materials/texture/chargedTexture.bmp");
	TextureID = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, spaceCraft.texture[1]);
	glUniform1i(TextureID, 1);


//==========================================================================
// O ring
//==========================================================================

	res = loadOBJ("Materials/Ring.obj", ring.vertices, ring.uvs, ring.normals);
	glGenVertexArrays(1, &ring.VAO);
	glBindVertexArray(ring.VAO);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, ring.vertices.size() * sizeof(glm::vec3), &ring.vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glGenBuffers(1, &uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, ring.uvs.size() * sizeof(glm::vec2), &ring.uvs[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, ring.normals.size() * sizeof(glm::vec2), &ring.normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	//Normal Mode
	ring.texture[0] = loadBMP_custom("Materials/texture/ringTexture.bmp");
	TextureID = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ring.texture[0]);
	glUniform1i(TextureID, 0);
	//Charging Mode
	ring.texture[1] = loadBMP_custom("Materials/texture/chargedTexture.bmp");
	TextureID = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ring.texture[1]);
	glUniform1i(TextureID, 1);
//=======================
// Rock
//=======================
	res = loadOBJ("Materials/rock.obj", rock.vertices, rock.uvs, rock.normals);
	glGenVertexArrays(1, &rock.VAO);
	glBindVertexArray(rock.VAO);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, rock.vertices.size() * sizeof(glm::vec3), &rock.vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glGenBuffers(1, &uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, rock.uvs.size() * sizeof(glm::vec2), &rock.uvs[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, rock.normals.size() * sizeof(glm::vec2), &rock.normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	//Normal Mode
	rock.texture[0] = loadBMP_custom("Materials/texture/rockTexture.bmp");
	TextureID = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, rock.texture[0]);
	glUniform1i(TextureID, 0);
	//==================


}

void paintGL(void)
{

	rotate_planet = rotate_planet + 0.05f;
	if (rotate_planet == 360.0f) {
		rotate_planet = 0;
	}

	if (glm::distance(glm::vec3(35.0f, 1.0f, 0.0f), glm::vec3(SC_x + Xmove * glm::radians(360.0f)*cos(Ymove*0.003f), 0.0f, SC_z - Xmove * glm::radians(360.0f)*sin(Ymove*0.003f))) < 4.0f) {
		passing_ring_3 = true;
	}
	if (glm::distance(glm::vec3(50.0f, 1.0f, 0.0f), glm::vec3(SC_x + Xmove * glm::radians(360.0f)*cos(Ymove*0.003f), 0.0f, SC_z - Xmove * glm::radians(360.0f)*sin(Ymove*0.003f))) < 4.0f) {
		passing_ring_2 = true;
	}
	if (glm::distance(glm::vec3(65.0f, 1.0f, 0.0f), glm::vec3(SC_x + Xmove * glm::radians(360.0f)*cos(Ymove*0.003f), 0.0f, SC_z - Xmove * glm::radians(360.0f)*sin(Ymove*0.003f))) < 4.0f) {
		passing_ring_1 = true;
	}
	glEnable(GL_CULL_FACE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//TODO:
	//Set lighting information, such as position and color of lighting source
	//Set transformation matrix
	//Bind different textures

	//glEnable(GL_TEXTURE_CUBE_MAP);
	glEnable(GL_TEXTURE_2D);
	glDepthMask(GL_FALSE);

	GLuint modelSkyBoxUniformLocation;
	GLuint modelProjectionMatrixUniformLocation;
	GLuint modelStatusMatrixUniformLocation;
	GLuint modelSKBXUniformLocation;
	glm::mat4 View;
	glm::mat4 modelProjectionMatrix;
	glm::mat4 modelScalingMatrix = glm::mat4(1.0f);
	glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
	glm::mat4 modelRotationMatrix = glm::mat4(1.0f);
	glm::mat4 modelStatusMatrix = glm::mat4(1.0f);

	GLuint eyePositionUniformLocation = glGetUniformLocation(programID, "eyePositionWorld");
	vec3 eyePosition(SC_x + Xmove * cos(Ymove), 2.0f, SC_z - Xmove * sin(Ymove));
	glUniform3fv(eyePositionUniformLocation, 1, &eyePosition[0]);

	glm::vec3 View_position = glm::vec3(Xmove, 0.0f, Zmove);
	GLint ambientLightUniformLocation = glGetUniformLocation(programID, "ambientLight");
	vec3 ambientLight(
		0.6f,	//R
		0.6f,	//G
		0.6f	//B

	);

	glUniform3fv(ambientLightUniformLocation, 1, &ambientLight[0]);
	
	GLint lightPositionUniformLocation = glGetUniformLocation(programID, "lightPositionWorld");
	vec3 lightPosition(0.0f, 0.0f, 30.0f);
	glUniform3fv(lightPositionUniformLocation, 1, &lightPosition[0]);

	GLint ambientLightUniformLocation2 = glGetUniformLocation(programID, "ambientLight2");
	vec3 ambientLight2(
		0.3f,	//R
		0.6f,	//G
		0.7f	//B

	);
	glUniform3fv(ambientLightUniformLocation, 1, &ambientLight2[0]);

	GLint lightPositionUniformLocation2 = glGetUniformLocation(programID, "lightPositionWorld2");
	vec3 lightPosition2(0.0f, 10.0f, 20.0f);
	glUniform3fv(lightPositionUniformLocation2, 1, &lightPosition2[0]);
	//===========================================================================
	// O Sky Box
	//===========================================================================
	glUseProgram(programID_SkyBox);
	glDisable(GL_CULL_FACE);
	modelSkyBoxUniformLocation = glGetUniformLocation(programID_SkyBox, "modelSkyBoxMatrix");
	glm::mat4 modelSkyBoxMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(modelSkyBoxUniformLocation, 1, GL_FALSE, &modelSkyBoxMatrix[0][0]);
	//view and projection

	View = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f + cos(Ymove), 0.0f, 0.0f + sin(Ymove)),
		glm::vec3(0.0f, -1.0f, 0.0f)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	//View = Eye.GetViewMatrix();
	modelProjectionMatrix = glm::perspective(45.0f, (float)GLUT_SCREEN_WIDTH / (float)GLUT_SCREEN_HEIGHT, 0.1f, 100.0f);

	modelProjectionMatrix = modelProjectionMatrix * View;

	modelProjectionMatrixUniformLocation = glGetUniformLocation(programID_SkyBox, "modelProjectionMatrix");
	glUniformMatrix4fv(modelProjectionMatrixUniformLocation, 1, GL_FALSE, &modelProjectionMatrix[0][0]);
	/*
	modelStatusMatrix = modelScalingMatrix * modelRotationMatrix * modelTransformMatrix;

	modelStatusMatrixUniformLocation = glGetUniformLocation(programID_SkyBox, "modelStatusMatrix");
	glUniformMatrix4fv(modelStatusMatrixUniformLocation, 1, GL_FALSE, &modelStatusMatrix[0][0]);
	*/
	//skybox cube
	glBindVertexArray(Skybox.VAO);
	glActiveTexture(GL_TEXTURE0);
	modelSKBXUniformLocation = glGetUniformLocation(programID_SkyBox, "SkyBox");
	glUniform1i(modelSKBXUniformLocation, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox.texture[0]);

	glDrawArrays(GL_TRIANGLES, 0, 36);
	//*/
	//glBindVertexArray(0);
//View set up
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	//glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	//glEnable(GL_CULL_FACE);

	//glEnable(GL_CULL_FACE);
	glUseProgram(programID);
	View = glm::lookAt(
		glm::vec3(SC_x, 2.0f, SC_z),											//Eye position
		glm::vec3(SC_x + Xmove * cos(Ymove), 2.0f, SC_z - Xmove * sin(Ymove)),	//View direction
		glm::vec3(0.0f, 1.0f, 0.0f)												// Head is up
	);

	modelProjectionMatrix = glm::perspective(glm::radians(45.0f), (float)Window_W / (float)Window_H, 0.1f, 150.0f);

	modelProjectionMatrix = modelProjectionMatrix * View;
	modelProjectionMatrixUniformLocation = glGetUniformLocation(programID, "modelProjectionMatrix");
	glUniformMatrix4fv(modelProjectionMatrixUniformLocation, 1, GL_FALSE, &modelProjectionMatrix[0][0]);
	GLint modelviewL = glGetUniformLocation(programID, "View");
	glUniformMatrix4fv(modelviewL, 1, GL_FALSE, &View[0][0]);
	//===========================================================================
	// O Planet
	//===========================================================================

		isEarth = true;
		modelTransformMatrix = glm::translate(glm::mat4(), glm::vec3(200.0f, 0.0f, 0.0f));
		modelScalingMatrix = glm::scale(glm::mat4(), glm::vec3(0.5f, 0.5f, 0.5f));
		modelRotationMatrix = glm::rotate(glm::mat4(), glm::radians(rotate_planet), glm::vec3(0, 1, 0));
		modelStatusMatrix = modelScalingMatrix * modelTransformMatrix * modelRotationMatrix;

		modelStatusMatrixUniformLocation = glGetUniformLocation(programID, "modelStatusMatrix");
		glUniformMatrix4fv(modelStatusMatrixUniformLocation, 1, GL_FALSE, &modelStatusMatrix[0][0]);
		doNormalMap = glGetUniformLocation(programID, "isEarth");
		glUniform1i(doNormalMap, isEarth);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Planet.texture[0]);
		glUniform1i(TextureID, 0);


		TextureID_n = glGetUniformLocation(programID, "myTextureSampler_n");
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, normTexture);
		glUniform1i(TextureID_n, 2);

		//Normal Mapping


		glBindVertexArray(Planet.VAO);
		glDrawArrays(GL_TRIANGLES, 0, Planet.vertices.size());
	//===========================================================================
	// O Wonder
	//===========================================================================
	isEarth = false;
	modelTransformMatrix = glm::translate(glm::mat4(), glm::vec3(20.0f, 0.0f, 0.0f));
	modelScalingMatrix = glm::scale(glm::mat4(), glm::vec3(0.5f, 0.5f, 0.5f));
	modelRotationMatrix = glm::rotate(glm::mat4(), glm::radians(rotate_planet), glm::vec3(0, 1, 0));
	modelStatusMatrix = modelScalingMatrix * modelTransformMatrix * modelRotationMatrix;

	modelStatusMatrixUniformLocation = glGetUniformLocation(programID, "modelStatusMatrix");
	glUniformMatrix4fv(modelStatusMatrixUniformLocation, 1, GL_FALSE, &modelStatusMatrix[0][0]);
	doNormalMap = glGetUniformLocation(programID, "isEarth");
	glUniform1i(doNormalMap, isEarth);
	glActiveTexture(GL_TEXTURE1);
	glBindVertexArray(Planet.VAO);
	glBindTexture(GL_TEXTURE_2D, Planet.texture[1]);
	glUniform1i(TextureID, 1);
	glDrawArrays(GL_TRIANGLES, 0, Planet.vertices.size());
	

	//===========================================================================
	// O spaceCraft
	//===========================================================================
	isEarth = false;
	glm::mat4 modelDifferentMatrix = glm::translate(glm::mat4(), glm::vec3(glm::radians(360.0f)*cos(Ymove*0.003f), 0.0f, -Xmove * glm::radians(360.0f)*sin(Ymove*0.003f)));
	glm::mat4 modelAngleMatrix = glm::rotate(glm::mat4(), glm::radians(90.0f), glm::vec3(0, 1, 0));
	modelTransformMatrix = glm::translate(glm::mat4(), glm::vec3(SC_x + Xmove * glm::radians(360.0f)*cos(Ymove*0.003f), 0.0f, SC_z - Xmove * glm::radians(360.0f)*sin(Ymove*0.003f)));
	modelScalingMatrix = glm::scale(glm::mat4(), glm::vec3(0.003f, 0.003f, 0.003f));
	modelRotationMatrix = glm::rotate(glm::mat4(), Ymove, glm::vec3(0, 1, 0));
	modelStatusMatrix = modelTransformMatrix * glm::inverse(modelDifferentMatrix) * modelRotationMatrix * modelDifferentMatrix *modelAngleMatrix* modelScalingMatrix;

	modelStatusMatrixUniformLocation = glGetUniformLocation(programID, "modelStatusMatrix");
	glUniformMatrix4fv(modelStatusMatrixUniformLocation, 1, GL_FALSE, &modelStatusMatrix[0][0]);
	if (passing_ring_3 || passing_ring_2 || passing_ring_1) {
		glActiveTexture(GL_TEXTURE1);
		glBindVertexArray(spaceCraft.VAO);
		glBindTexture(GL_TEXTURE_2D, spaceCraft.texture[1]);
		glUniform1i(TextureID, 1);
		glDrawArrays(GL_TRIANGLES, 0, spaceCraft.vertices.size());
	}
	else {
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(spaceCraft.VAO);
		glBindTexture(GL_TEXTURE_2D, spaceCraft.texture[0]);
		glUniform1i(TextureID, 0);
		glDrawArrays(GL_TRIANGLES, 0, spaceCraft.vertices.size());
	}
	doNormalMap = glGetUniformLocation(programID, "isEarth");
	glUniform1i(doNormalMap, isEarth);
	//===========================================================================
	// O Ring 1
	//===========================================================================
	isEarth = false;
	modelAngleMatrix = glm::rotate(glm::mat4(), glm::radians(90.0f), glm::vec3(0, 0, 1));
	modelTransformMatrix = glm::translate(glm::mat4(), glm::vec3(1.0f, -60.0f, 0.0f));
	modelScalingMatrix = glm::scale(glm::mat4(), glm::vec3(0.045f, 0.045f, 0.045f));
	modelRotationMatrix = glm::rotate(glm::mat4(), glm::radians(rotate_planet * (0.5f)), glm::vec3(1, 0, 0));
	modelStatusMatrix = modelAngleMatrix * modelTransformMatrix * modelRotationMatrix * modelScalingMatrix;

	modelStatusMatrixUniformLocation = glGetUniformLocation(programID, "modelStatusMatrix");
	glUniformMatrix4fv(modelStatusMatrixUniformLocation, 1, GL_FALSE, &modelStatusMatrix[0][0]);
	if (passing_ring_1) {
		glActiveTexture(GL_TEXTURE1);
		glBindVertexArray(ring.VAO);
		glBindTexture(GL_TEXTURE_2D, ring.texture[1]);
		glUniform1i(TextureID, 1);
		glDrawArrays(GL_TRIANGLES, 0, ring.vertices.size());
		passing_ring_1 = false;
	}
	else {
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(ring.VAO);
		glBindTexture(GL_TEXTURE_2D, ring.texture[0]);
		glUniform1i(TextureID, 0);
		glDrawArrays(GL_TRIANGLES, 0, ring.vertices.size());
	}
	doNormalMap = glGetUniformLocation(programID, "isEarth");
	glUniform1i(doNormalMap, isEarth);
	//===========================================================================
	// O Ring 2
	//===========================================================================
	isEarth = false;
	modelAngleMatrix = glm::rotate(glm::mat4(), glm::radians(90.0f), glm::vec3(0, 0, 1));
	modelTransformMatrix = glm::translate(glm::mat4(), glm::vec3(1.0f, -60.0f + 15.0f, 0.0f));
	modelScalingMatrix = glm::scale(glm::mat4(), glm::vec3(0.045f, 0.045f, 0.045f));
	modelRotationMatrix = glm::rotate(glm::mat4(), glm::radians(rotate_planet * (0.5f - 0.1f)), glm::vec3(1, 0, 0));
	modelStatusMatrix = modelAngleMatrix * modelTransformMatrix * modelRotationMatrix * modelScalingMatrix;

	modelStatusMatrixUniformLocation = glGetUniformLocation(programID, "modelStatusMatrix");
	glUniformMatrix4fv(modelStatusMatrixUniformLocation, 1, GL_FALSE, &modelStatusMatrix[0][0]);
	if (passing_ring_2) {
		glActiveTexture(GL_TEXTURE1);
		glBindVertexArray(ring.VAO);
		glBindTexture(GL_TEXTURE_2D, ring.texture[1]);
		glUniform1i(TextureID, 1);
		glDrawArrays(GL_TRIANGLES, 0, ring.vertices.size());
		passing_ring_2 = false;
	}
	else {
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(ring.VAO);
		glBindTexture(GL_TEXTURE_2D, ring.texture[0]);
		glUniform1i(TextureID, 0);
		glDrawArrays(GL_TRIANGLES, 0, ring.vertices.size());
	}
	doNormalMap = glGetUniformLocation(programID, "isEarth");
	glUniform1i(doNormalMap, isEarth);
	//===========================================================================
	// O Ring 3
	//===========================================================================
	isEarth = false;
	modelAngleMatrix = glm::rotate(glm::mat4(), glm::radians(90.0f), glm::vec3(0, 0, 1));
	modelTransformMatrix = glm::translate(glm::mat4(), glm::vec3(1.0f, -60.0f + 30.0f, 0.0f));
	modelScalingMatrix = glm::scale(glm::mat4(), glm::vec3(0.045f, 0.045f, 0.045f));
	modelRotationMatrix = glm::rotate(glm::mat4(), glm::radians(rotate_planet * (0.5f - 0.2f)), glm::vec3(1, 0, 0));
	modelStatusMatrix = modelAngleMatrix * modelTransformMatrix * modelRotationMatrix * modelScalingMatrix;

	modelStatusMatrixUniformLocation = glGetUniformLocation(programID, "modelStatusMatrix");
	glUniformMatrix4fv(modelStatusMatrixUniformLocation, 1, GL_FALSE, &modelStatusMatrix[0][0]);

	if (passing_ring_3) {
		glActiveTexture(GL_TEXTURE1);
		glBindVertexArray(ring.VAO);
		glBindTexture(GL_TEXTURE_2D, ring.texture[1]);
		glUniform1i(TextureID, 1);
		glDrawArrays(GL_TRIANGLES, 0, ring.vertices.size());
		passing_ring_3 = false;
	}
	else {
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(ring.VAO);
		glBindTexture(GL_TEXTURE_2D, ring.texture[0]);
		glUniform1i(TextureID, 0);
		glDrawArrays(GL_TRIANGLES, 0, ring.vertices.size());
	}
	doNormalMap = glGetUniformLocation(programID, "isEarth");
	glUniform1i(doNormalMap, isEarth);
	//==============
	//	rock
	//============
	
	glm::mat4 Rock_Orbit_T = glm::translate(glm::mat4(), glm::vec3(10.0f, 0.0f, 0.0f));
	glm::mat4 Rock_Orbit_R = glm::rotate(Rock_Orbit_T, Rock_Orbit_angle, glm::vec3(0, 1, 0));
	Rock_Orbit_angle += 0.002f;
	if (Rock_Orbit_angle == 360.0f) {
		Rock_Orbit_angle = 0;
	}
	for (int i = 0; i < 250; i++) {
		modelStatusMatrix = Rock_Orbit_R * Rock_Matrices[i];
		modelStatusMatrixUniformLocation = glGetUniformLocation(programID, "modelStatusMatrix");
		glUniformMatrix4fv(modelStatusMatrixUniformLocation, 1, GL_FALSE, &modelStatusMatrix[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(rock.VAO);
		glBindTexture(GL_TEXTURE_2D, rock.texture[0]);
		glUniform1i(TextureID, 0);
		glDrawArrays(GL_TRIANGLES, 0, rock.vertices.size());
		doNormalMap = glGetUniformLocation(programID, "isEarth");
		glUniform1i(doNormalMap, isEarth);
	}
	
	


	glDisable(GL_DEPTH_TEST);

	glFlush();
	glutPostRedisplay();
}

void initializedGL(void) //run only once
{
	glClearColor(0.1f, 0.5f, 0.5f, 1.0f);
	glewInit();
	installShaders();
	installSkyBoxShaders();
	CreateRand_ModelM();
	sendDataToOpenGL();
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitWindowSize(Window_W,Window_H);
	glutCreateWindow("Final Project");
	
	//TODO:
	/*Register different CALLBACK function for GLUT to response
	with different events, e.g. window sizing, mouse click or
	keyboard stroke */
	
	initializedGL();
	glutDisplayFunc(paintGL);

	glutKeyboardFunc(keyboard);
	glutSpecialFunc(move);
	glutPassiveMotionFunc(PassiveMouse);

	glutMainLoop();

	return 0;
}