/*
Student Information
Student ID:	1155125534 1155126240
Student Name: Cheng Ka Pui & Lam Puy Yin
*/

#include "Dependencies/glew/glew.h"
#include "Dependencies/GLFW/glfw3.h"
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
//#include "Dependencies/freeglut/freeglut.h"

#include "Dependencies/stb_image/stb_image.h"
#define STB_IMAGE_IMPLEMENTATION

#include "Shader.h"
#include "Texture.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

GLint programID;
// screen setting
const int SCR_WIDTH = 2400;
const int SCR_HEIGHT = 1800;

int textureKey[3] = { 0 , 0 , 0 };
int ufo_textureKey[3] = { 0 , 0 , 0 };

float theta =20.0f;

float lightValue = 0.0;
float lightValue2 = 0.0;
int lightValue4 = 0;
float lightValue5 = 0.0;
float zoom = 45.0f;

int amount = 1000;

glm::mat4 spaceshipLocal = glm::mat4(1.0f);
glm::mat4 spaceship_Rot_M = glm::mat4(1.0f);
float SCInitialPos[3] = { 50.0f, 0.0f, 50.0f };
float SCTranslation[3] = { 1.0f, 0.0f, 1.0f };
glm::vec4 cameraTarget, SC_world_Front_Direction, SC_world_Right_Direction;
glm::vec3 SC_local_front = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 SC_local_right = glm::vec3(-1.0f, 0.0f, 0.0f);

glm::vec4 cameraLocation;
glm::vec3 cameraFront = glm::vec3(0, 400, -1700);

// struct for storing the obj file
struct Vertex {
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
};

struct Model {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};


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
		std::cout << buffer << std::endl;

		delete[] buffer;
		return false;
	}
	return true;
}


bool checkShaderStatus(GLuint shaderID) {
	return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool checkProgramStatus(GLuint programID) {
	return checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

std::string readShaderCode(const char* fileName) {
	std::ifstream meInput(fileName);
	if (!meInput.good()) {
		std::cout << "File failed to load ... " << fileName << std::endl;
		exit(1);
	}
	return std::string(
		std::istreambuf_iterator<char>(meInput),
		std::istreambuf_iterator<char>()
	);
}

void installShaders() {
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	//adapter[0] = vertexShaderCode;
	std::string temp = readShaderCode("VertexShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	//adapter[0] = fragmentShaderCode;
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

Model loadOBJ(const char* objPath)
{
	// function to load the obj file
	// Note: this simple function cannot load all obj files.

	struct V {
		// struct for identify if a vertex has showed up
		unsigned int index_position, index_uv, index_normal;
		bool operator == (const V& v) const {
			return index_position == v.index_position && index_uv == v.index_uv && index_normal == v.index_normal;
		}
		bool operator < (const V& v) const {
			return (index_position < v.index_position) ||
				(index_position == v.index_position && index_uv < v.index_uv) ||
				(index_position == v.index_position && index_uv == v.index_uv && index_normal < v.index_normal);
		}
	};

	std::vector<glm::vec3> temp_positions;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	std::map<V, unsigned int> temp_vertices;

	Model model;
	unsigned int num_vertices = 0;

	std::cout << "\nLoading OBJ file " << objPath << "..." << std::endl;

	std::ifstream file;
	file.open(objPath);

	// Check for Error
	if (file.fail()) {
		std::cerr << "Impossible to open the file! Do you use the right path? See Tutorial 6 for details" << std::endl;
		exit(1);
	}

	while (!file.eof()) {
		// process the object file
		char lineHeader[128];
		file >> lineHeader;

		if (strcmp(lineHeader, "v") == 0) {
			// geometric vertices
			glm::vec3 position;
			file >> position.x >> position.y >> position.z;
			temp_positions.push_back(position);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			// texture coordinates
			glm::vec2 uv;
			//fscanf(file, "%f %f\n", &uv.x, &uv.y);
			//uv.y = -uv.y;
			file >> uv.x >> uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			// vertex normals
			glm::vec3 normal;
			file >> normal.x >> normal.y >> normal.z;
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			// Face elements
			V vertices[3];
			for (int i = 0; i < 3; i++) {
				char ch;
				file >> vertices[i].index_position >> ch >> vertices[i].index_uv >> ch >> vertices[i].index_normal;
			}

			// Check if there are more than three vertices in one face.
			std::string redundency;
			std::getline(file, redundency);
			if (redundency.length() >= 5) {
				std::cerr << "There may exist some errors while load the obj file. Error content: [" << redundency << " ]" << std::endl;
				std::cerr << "Please note that we only support the faces drawing with triangles. There are more than three vertices in one face." << std::endl;
				std::cerr << "Your obj file can't be read properly by our simple parser :-( Try exporting with other options." << std::endl;
				exit(1);
			}

			for (int i = 0; i < 3; i++) {
				if (temp_vertices.find(vertices[i]) == temp_vertices.end()) {
					// the vertex never shows before
					Vertex vertex;
					vertex.position = temp_positions[vertices[i].index_position - 1];
					vertex.uv = temp_uvs[vertices[i].index_uv - 1];
					vertex.normal = temp_normals[vertices[i].index_normal - 1];

					model.vertices.push_back(vertex);
					model.indices.push_back(num_vertices);
					temp_vertices[vertices[i]] = num_vertices;
					num_vertices += 1;
				}
				else {
					// reuse the existing vertex
					unsigned int index = temp_vertices[vertices[i]];
					model.indices.push_back(index);
				}
			} // for
		} // else if
		else {
			// it's not a vertex, texture coordinate, normal or face
			char stupidBuffer[1024];
			file.getline(stupidBuffer, 1024);
		}
	}
	file.close();

	std::cout << "There are " << num_vertices << " vertices in the obj file.\n" << std::endl;
	return model;
}

void get_OpenGL_info()
{
	// OpenGL information
	const GLubyte* name = glGetString(GL_VENDOR);
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* glversion = glGetString(GL_VERSION);
	std::cout << "OpenGL company: " << name << std::endl;
	std::cout << "Renderer name: " << renderer << std::endl;
	std::cout << "OpenGL version: " << glversion << std::endl;
}

GLuint loadTexture(const char* texturePath)
{
	stbi_set_flip_vertically_on_load(true);
	int Width, Height, BPP;
	unsigned char* data = stbi_load(texturePath, &Width, &Height, &BPP, 0);

	GLenum format = 3;
	switch (BPP) {
	case 1: format = GL_RED; break;
	case 3: format = GL_RGB; break;
	case 4: format = GL_RGBA; break;
	}
	if (!data) {
		std::cout << "Failed to load texture: " << texturePath << std::endl;
		exit(1);
	}

	GLuint textureID = 0;

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);
	std::cout << "Load " << texturePath << " successfully!" << std::endl;
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}




GLuint vao[20];
GLuint vbo[20];
GLuint ebo[20];
Model obj[20];

GLuint spacecraftTexture[2];
GLuint planetTexture[2];
GLuint alienvehicleTexture[2];
GLuint alienTexture;
GLuint rockTexture;
GLuint chickenTexture;
GLuint appleTexture;

GLuint loadTexture(const char* texturePath);

void loadspacecraft() {
	// load the spacecraft data from the OBJ file 

	obj[0] = loadOBJ("resources/spacecraft/spacecraft.obj");
	spacecraftTexture[0] = loadTexture("resources/spacecraft/spacecraftTexture.bmp");
	spacecraftTexture[1] = loadTexture("resources/spacecraft/leisure_spacecraftTexture.bmp");


	glGenVertexArrays(1, &vao[0]);
	glBindVertexArray(vao[0]);

	glGenBuffers(1, &vbo[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, obj[0].vertices.size() * sizeof(Vertex), &obj[0].vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &ebo[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj[0].indices.size() * sizeof(unsigned int), &obj[0].indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
}

void loadplanet() {
	// load the planet data from the OBJ file 

	obj[1] = loadOBJ("resources/planet/planet.obj");
	planetTexture[0] = loadTexture("resources/planet/planetTexture.bmp");
	planetTexture[1] = loadTexture("resources/planet/planetNormal.bmp");


	glGenVertexArrays(1, &vao[1]);
	glBindVertexArray(vao[1]);

	glGenBuffers(1, &vbo[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, obj[1].vertices.size() * sizeof(Vertex), &obj[1].vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &ebo[1]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj[1].indices.size() * sizeof(unsigned int), &obj[1].indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
}

void loadalienvehicle() {

	obj[2] = loadOBJ("resources/alien/alienvehicle.obj");
	alienvehicleTexture[0]= loadTexture("resources/alien/alienTexture.bmp");
	alienvehicleTexture[1]= loadTexture("resources/alien/colorful_alien_vehicleTexture.bmp");
	
	glGenVertexArrays(1, &vao[2]);
	glBindVertexArray(vao[2]);

	glGenBuffers(1, &vbo[2]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, obj[2].vertices.size() * sizeof(Vertex), &obj[2].vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &ebo[2]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj[2].indices.size() * sizeof(unsigned int), &obj[2].indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
}


void loadalien() {
	obj[3] = loadOBJ("resources/alien/alienpeople.obj");
	alienTexture = loadTexture("resources/alien/alienTexture.bmp");
	glGenVertexArrays(1, &vao[3]);
	glBindVertexArray(vao[3]);
	glGenBuffers(1, &vbo[3]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, obj[3].vertices.size() * sizeof(Vertex), &obj[3].vertices[0], GL_STATIC_DRAW);
	glGenBuffers(1, &ebo[3]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj[3].indices.size() * sizeof(unsigned int), &obj[3].indices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
}

void loadrock() {
	obj[4] = loadOBJ("resources/rock/rock.obj");
	rockTexture = loadTexture("resources/rock/rockTexture.bmp");
	glGenVertexArrays(1, &vao[4]);
	glBindVertexArray(vao[4]);
	glGenBuffers(1, &vbo[4]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glBufferData(GL_ARRAY_BUFFER, obj[4].vertices.size() * sizeof(Vertex), &obj[4].vertices[0], GL_STATIC_DRAW);
	glGenBuffers(1, &ebo[4]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[4]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj[4].indices.size() * sizeof(unsigned int), &obj[4].indices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
}

void loadchicken() {
	obj[5] = loadOBJ("resources/chicken/chicken.obj");
	chickenTexture = loadTexture("resources/chicken/chickenTexture.bmp");
	glGenVertexArrays(1, &vao[5]);
	glBindVertexArray(vao[5]);
	glGenBuffers(1, &vbo[5]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glBufferData(GL_ARRAY_BUFFER, obj[5].vertices.size() * sizeof(Vertex), &obj[5].vertices[0], GL_STATIC_DRAW);
	glGenBuffers(1, &ebo[5]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[5]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj[5].indices.size() * sizeof(unsigned int), &obj[5].indices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
}

void loadapple() {
	obj[6] = loadOBJ("resources/apple/apple.obj");
	appleTexture = loadTexture("resources/apple/appleTexture.png");
	glGenVertexArrays(1, &vao[6]);
	glBindVertexArray(vao[6]);
	glGenBuffers(1, &vbo[6]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
	glBufferData(GL_ARRAY_BUFFER, obj[6].vertices.size() * sizeof(Vertex), &obj[6].vertices[0], GL_STATIC_DRAW);
	glGenBuffers(1, &ebo[6]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[6]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj[6].indices.size() * sizeof(unsigned int), &obj[6].indices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
}

void sendDataToOpenGL()
{
	//TODO
	//Load objects and bind to VAO and VBO
	//Load textures

	loadspacecraft();

	loadplanet();

	loadalienvehicle();

	loadalien();

	loadrock();

	loadchicken();

	loadapple();
}

void initializedGL(void) //run only once
{
	if (glewInit() != GLEW_OK) {
		std::cout << "GLEW not OK." << std::endl;
	}

	get_OpenGL_info();
	sendDataToOpenGL();

	//TODO: set up the camera parameters
	//TODO: set up the vertex shader and fragment shader
	installShaders();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}


//initialize light properties

void materialLight()
{
	glm::vec3 materialambient = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 materialdiffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 materialspecular = glm::vec3(1.0f, 1.0f, 1.0f);
	float materialshininess = 100.0f;
	GLint materialambientUniformLocation = glGetUniformLocation(programID, "materialambient");
	GLint materialdiffuseUniformLocation = glGetUniformLocation(programID, "materialdiffuse");
	GLint materialspecularUniformLocation = glGetUniformLocation(programID, "materialspecular");
	GLint materialshininessUniformLocation = glGetUniformLocation(programID, "materialshininess");
	glUniform3fv(materialambientUniformLocation, 1, &materialambient[0]);
	glUniform3fv(materialdiffuseUniformLocation, 1, &materialdiffuse[0]);
	glUniform3fv(materialspecularUniformLocation, 1, &materialspecular[0]);
	glUniform1f(materialshininessUniformLocation, materialshininess);
}

void pointLight()
{
	float colorForPL = ((sin((float)glfwGetTime())+1)/2) * 300;
	glm::vec3 pointLightposition = glm::vec3(10.0f, 10.0f, 1.0f);
	float pointLightconstant = 1.0f;
	float pointLightlinear = 0.09f;
	float pointLightquadratic = 0.32f;
	glm::vec3 pointLightcolor = glm::vec3((50.0f + colorForPL) * lightValue5, (10.0f + colorForPL) * lightValue5, 10.0f * lightValue5);
	GLint pointLightpositionUniformLocation = glGetUniformLocation(programID, "pointLightposition");
	GLint pointLightconstantUniformLocation = glGetUniformLocation(programID, "pointLightconstant");
	GLint pointLightlinearUniformLocation = glGetUniformLocation(programID, "pointLightlinear");
	GLint pointLightquadraticUniformLocation = glGetUniformLocation(programID, "pointLightquadratic");
	GLint pointLightcolorUniformLocation = glGetUniformLocation(programID, "pointLightcolor");
	glUniform3fv(pointLightpositionUniformLocation, 1, &pointLightposition[0]);
	glUniform1f(pointLightconstantUniformLocation, pointLightconstant);
	glUniform1f(pointLightlinearUniformLocation, pointLightlinear);
	glUniform1f(pointLightquadraticUniformLocation, pointLightquadratic);
	glUniform3fv(pointLightcolorUniformLocation, 1, &pointLightcolor[0]);
}

void spotLight()
{
	glm::vec3 spotLightposition = glm::vec3(-40.0f + lightValue2, 5.0f, 5.0f);
	glm::vec3 spotLightdirection = glm::vec3(0.5f, 0.5f, 0.5f);
	float spotLightcutoff = glm::cos(glm::radians(45.0f));
	float spotLightouterCutOff = glm::cos(glm::radians(15.0f));
	float spotLightconstant = 1.0f;
	float spotLightlinear = 0.0006f;
	float spotLightquadratic = 0.0016f;
	glm::vec3 spotLightcolor = glm::vec3(5.0f * lightValue4, 5.0f * lightValue4, 5.0f * lightValue4);
	GLint spotLightpositionUniformLocation = glGetUniformLocation(programID, "spotLightposition");
	GLint spotLightdirectionUniformLocation = glGetUniformLocation(programID, "spotLightdirection");
	GLint spotLightcutoffUniformLocation = glGetUniformLocation(programID, "spotLightcutoff");
	GLint spotLightouterCutOffUniformLocation = glGetUniformLocation(programID, "spotLightouterCutOff");
	GLint spotLightconstantUniformLocation = glGetUniformLocation(programID, "spotLightconstant");
	GLint spotLightlinearUniformLocation = glGetUniformLocation(programID, "spotLightlinear");
	GLint spotLightquadraticUniformLocation = glGetUniformLocation(programID, "spotLightquadratic");
	GLint spotLightcolorUniformLocation = glGetUniformLocation(programID, "spotLightcolor");
	glUniform3fv(spotLightpositionUniformLocation, 1, &spotLightposition[0]);
	glUniform3fv(spotLightdirectionUniformLocation, 1, &spotLightdirection[0]);
	glUniform1f(spotLightcutoffUniformLocation, spotLightcutoff);
	glUniform1f(spotLightouterCutOffUniformLocation, spotLightouterCutOff);
	glUniform1f(spotLightconstantUniformLocation, spotLightconstant);
	glUniform1f(spotLightlinearUniformLocation, spotLightlinear);
	glUniform1f(spotLightquadraticUniformLocation, spotLightquadratic);
	glUniform3fv(spotLightcolorUniformLocation, 1, &spotLightcolor[0]);
}

void directionalLight()
{
	glm::vec3 dirLightcolor = glm::vec3(0.8f + lightValue);
	GLint dirLightBrightness = glGetUniformLocation(programID, "dirLightBrightness");
	glUniform3fv(dirLightBrightness, 1, &dirLightcolor[0]);

	glm::vec3 dirLightdirection = glm::vec3(0.0f, -10.0f, 0.0f);
	GLint dirLightdirectionUniformLocation = glGetUniformLocation(programID, "dirLightdirection");
	glUniform3fv(dirLightdirectionUniformLocation, 1, &dirLightdirection[0]);
}

void ambientLight()
{
	GLint ambientLightingUniformLocation = glGetUniformLocation(programID, "ambientLight");
	float ambientLight = 0.9f;
	glUniform1f(ambientLightingUniformLocation, ambientLight);
}


void specularLight()
{
	GLint eyePositionUniformLocation = glGetUniformLocation(programID, "cameraPosition");
	glUniform3fv(eyePositionUniformLocation, 1, &cameraTarget[0]);
}


//camera and spaceship system update
void UpdateStatus() {
	float scale = 0.005;

	glm::mat4 spaceship_scale_M = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
	glm::mat4 spaceship_trans_M = glm::translate(glm::mat4(1.0f),
		glm::vec3(SCInitialPos[0] + SCTranslation[0], SCInitialPos[1] + SCTranslation[1],
			SCInitialPos[2] + SCTranslation[2]));
	spaceship_Rot_M = glm::rotate(glm::mat4(1.0f), glm::radians(theta), glm::vec3(0.0f, 1.0f, 0.0f));

	spaceshipLocal = spaceship_trans_M * spaceship_Rot_M * spaceship_scale_M;
	cameraTarget = spaceshipLocal * glm::vec4(SC_local_front, 1.0f);
	SC_world_Front_Direction = spaceshipLocal * glm::vec4(SC_local_front, 1.0f);
	SC_world_Right_Direction = spaceshipLocal * glm::vec4(SC_local_right, 1.0f);
	SC_world_Front_Direction = normalize(SC_world_Front_Direction);
	SC_world_Right_Direction = normalize(SC_world_Right_Direction);
	cameraLocation = spaceshipLocal * glm::vec4(cameraFront, 1.0f);
}



// obj and camera initialize

//earth size scale = 3
glm::vec3 earth_location = glm::vec3(300.0f/3.0, 0.0f, 300.0f/3.0);

// chicken size scale = 0.01
glm::vec3 chicken_location = glm::vec3(95.0f / 0.01, 0.0f, 90.0f / 0.01);
float chicken_trigger = 1.0f;

// apple size scale = 1
glm::vec3 apple_location_1 = glm::vec3(140.0f, 1.0f, 145.0f);
float apple_trigger_1 = 1.0f;

glm::vec3 apple_location_2 = glm::vec3(190.0f, 1.0f, 195.0f);
float apple_trigger_2 = 1.0f;


// ufo size scale = 0.5
glm::vec3 ufo_location_1 = glm::vec3(100.0f / 0.5, 0.0f, 100.0f / 0.5);
glm::vec3 ufo_location_2 = glm::vec3(150.0f / 0.5, 0.0f, 150.0f / 0.5);
glm::vec3 ufo_location_3 = glm::vec3(200.0f / 0.5, 0.0f, 200.0f / 0.5);

// aliean size scale = 0.4
glm::vec3 alien_location_1 = glm::vec3(100.0f / 0.4, 0.0f, 100.0f / 0.4);
glm::vec3 alien_location_2 = glm::vec3(150.0f / 0.4, 0.0f, 150.0f / 0.4);
glm::vec3 alien_location_3 = glm::vec3(200.0f / 0.4, 0.0f, 200.0f / 0.4);


void drawObj(int objNum) {
	GLint modelTransformMatrixUniformLocation;
	glm::mat4 modelTransformMatrix;
	GLuint slot;
	GLuint TextureID;

	

	if (objNum == 1) {   //spacecraft
		slot = 0;
		TextureID = glGetUniformLocation(programID, "myTextureSampler");
		glActiveTexture(GL_TEXTURE0 + slot);
		if (textureKey[0] == 0)
			glBindTexture(GL_TEXTURE_2D, spacecraftTexture[0]);
		else
			glBindTexture(GL_TEXTURE_2D, spacecraftTexture[1]);

		glUniform1i(TextureID, slot);

		modelTransformMatrix = spaceshipLocal;


		modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
		glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	}

	if (objNum == 2) {   //planet
		slot = 0;
		TextureID = glGetUniformLocation(programID, "myTextureSampler");
		glActiveTexture(GL_TEXTURE0 + slot);
		if (textureKey[1] == 0)
			glBindTexture(GL_TEXTURE_2D, planetTexture[0]);
		else
			glBindTexture(GL_TEXTURE_2D, planetTexture[1]);

		glUniform1i(TextureID, slot);

		modelTransformMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(3.0f));;
		modelTransformMatrix *= glm::translate(glm::mat4(1.0f), earth_location);
		modelTransformMatrix *= glm::rotate(glm::mat4(1.0f), glm::radians((float)glfwGetTime() * 100), glm::vec3(0, 1, 0));;

		modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
		glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	}

	if (objNum == 3) {   //rock
		slot = 0;
		TextureID = glGetUniformLocation(programID, "myTextureSampler");
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, rockTexture);
		glUniform1i(TextureID, slot);

		modelTransformMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0, 1.0, 1.0));;
		modelTransformMatrix *= glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.0f, 0.0f));;
		modelTransformMatrix *= glm::rotate(glm::mat4(1.0f), glm::radians((float)glfwGetTime() * 100), glm::vec3(0, 1, 0));;
		modelTransformMatrix *= glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f));;

		modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
		glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	}

	if (objNum == 4) {   //chicken
		slot = 0;
		TextureID = glGetUniformLocation(programID, "myTextureSampler");
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, chickenTexture);
		glUniform1i(TextureID, slot);

		modelTransformMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f * chicken_trigger));;
		modelTransformMatrix *= glm::translate(glm::mat4(1.0f), chicken_location);;
		modelTransformMatrix *= glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));;
		modelTransformMatrix *= glm::rotate(glm::mat4(1.0f), glm::radians((float)glfwGetTime() * 100), glm::vec3(0, 0, 1));;

		modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
		glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	}

	if (objNum == 5) {   //apple1

		slot = 0;
		TextureID = glGetUniformLocation(programID, "myTextureSampler");
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, appleTexture);
		glUniform1i(TextureID, slot);

		modelTransformMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f* apple_trigger_1));;
		modelTransformMatrix *= glm::translate(glm::mat4(1.0f), apple_location_1);;
		modelTransformMatrix *= glm::rotate(glm::mat4(1.0f), glm::radians((float)glfwGetTime() * 100), glm::vec3(1, 1, 1));;


		modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
		glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	}

	if (objNum == 6) {   //alienvehicle1
		slot = 0;
		TextureID = glGetUniformLocation(programID, "myTextureSampler");
		glActiveTexture(GL_TEXTURE0 + slot);

		if (ufo_textureKey[0] == 0)
			glBindTexture(GL_TEXTURE_2D, alienvehicleTexture[0]);
		else
			glBindTexture(GL_TEXTURE_2D, alienvehicleTexture[1]);

		glUniform1i(TextureID, slot);

		modelTransformMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.5));;
		modelTransformMatrix *= glm::translate(glm::mat4(1.0f), glm::vec3(ufo_location_1));;
		modelTransformMatrix *= glm::rotate(glm::mat4(1.0f), glm::radians((float)glfwGetTime() * 100), glm::vec3(0, 1, 0));;

		modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
		glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	}

	if (objNum == 7) {   //alienvehicle2
		slot = 0;
		TextureID = glGetUniformLocation(programID, "myTextureSampler");
		glActiveTexture(GL_TEXTURE0 + slot);
		if (ufo_textureKey[1] == 0)
			glBindTexture(GL_TEXTURE_2D, alienvehicleTexture[0]);
		else
			glBindTexture(GL_TEXTURE_2D, alienvehicleTexture[1]);
		glUniform1i(TextureID, slot);

		modelTransformMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.5));;
		modelTransformMatrix *= glm::translate(glm::mat4(1.0f), glm::vec3(ufo_location_2));;
		modelTransformMatrix *= glm::rotate(glm::mat4(1.0f), glm::radians((float)glfwGetTime() * 100), glm::vec3(0, 1, 0));;

		modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
		glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	}

	if (objNum == 8) {   //alienvehicle3
		slot = 0;
		TextureID = glGetUniformLocation(programID, "myTextureSampler");
		glActiveTexture(GL_TEXTURE0 + slot);
		if (ufo_textureKey[2] == 0)
			glBindTexture(GL_TEXTURE_2D, alienvehicleTexture[0]);
		else
			glBindTexture(GL_TEXTURE_2D, alienvehicleTexture[1]);
		glUniform1i(TextureID, slot);

		modelTransformMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.5));;
		modelTransformMatrix *= glm::translate(glm::mat4(1.0f), glm::vec3(ufo_location_3));;
		modelTransformMatrix *= glm::rotate(glm::mat4(1.0f), glm::radians((float)glfwGetTime() * 100), glm::vec3(0, 1, 0));;

		modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
		glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	}

	if (objNum == 9) {   //alien1
		slot = 0;
		TextureID = glGetUniformLocation(programID, "myTextureSampler");
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, alienTexture);
		glUniform1i(TextureID, slot);

		modelTransformMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.4));;
		modelTransformMatrix *= glm::translate(glm::mat4(1.0f), glm::vec3(alien_location_1));;

		float movement = glm::radians((float)glfwGetTime() * 100);
		modelTransformMatrix *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, (7/ 0.4) * 0.4 + (sin(movement+90) * 2), 0.0f));;

		modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
		glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	}

	if (objNum == 10) {   //alien2
		slot = 0;
		TextureID = glGetUniformLocation(programID, "myTextureSampler");
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, alienTexture);
		glUniform1i(TextureID, slot);

		modelTransformMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.4));;
		modelTransformMatrix *= glm::translate(glm::mat4(1.0f), glm::vec3(alien_location_2));;

		float movement = glm::radians((float)glfwGetTime() * 100);
		modelTransformMatrix *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, (7/0.4) * 0.4 + (sin(movement) * 2), 0.0f));;

		modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
		glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	}

	if (objNum == 11) {   //alien3
		slot = 0;
		TextureID = glGetUniformLocation(programID, "myTextureSampler");
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, alienTexture);
		glUniform1i(TextureID, slot);

		modelTransformMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.4));;
		modelTransformMatrix *= glm::translate(glm::mat4(1.0f), glm::vec3(alien_location_3));;

		float movement = glm::radians((float)glfwGetTime() * 100);
		modelTransformMatrix *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, (7 / 0.4) * 0.4 + (sin(movement+180) * 2), 0.0f));;

		modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
		glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	}

	if (objNum == 12) {   //apple2

		slot = 0;
		TextureID = glGetUniformLocation(programID, "myTextureSampler");
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, appleTexture);
		glUniform1i(TextureID, slot);

		modelTransformMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f * apple_trigger_2));;
		modelTransformMatrix *= glm::translate(glm::mat4(1.0f), apple_location_2);;
		modelTransformMatrix *= glm::rotate(glm::mat4(1.0f), glm::radians((float)glfwGetTime() * 150 +180), glm::vec3(1, 0, 1));;


		modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
		glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	}

	//view camera	
	glm::mat4 viewMatrix = glm::mat4(1.0f);
	viewMatrix = glm::lookAt(glm::vec3(cameraLocation), glm::vec3(cameraTarget), glm::vec3(0.0f, 1.0f, 0.0f));
	
	GLint viewMatrixUniformLocation =
		glGetUniformLocation(programID, "viewMatrix");
	glUniformMatrix4fv(viewMatrixUniformLocation, 1,
		GL_FALSE, &viewMatrix[0][0]);

	//projection
	glm::mat4 projectionMatrix;
	projectionMatrix = glm::perspective(glm::radians(zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
	GLint projectionMatrixUniformLocation = glGetUniformLocation(programID, "projectionMatrix");
	glUniformMatrix4fv(projectionMatrixUniformLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
}

void Asteroids(int amount) {
	glm::mat4* modelMatrices;
	modelMatrices = new glm::mat4[amount];
	srand(glfwGetTime() * 0.000001); // initialize random seed
	float radius = 16.5;
	float offset = 4.5f;

	for (GLuint i = 0; i < amount; i++)
	{
		glm::mat4 model = glm::mat4(1.0f);

		// rotate at earth centre
		model = glm::translate(model, glm::vec3(earth_location.x * 3, earth_location.y, earth_location.z * 3));
		model = glm::rotate(model, glm::radians((float)glfwGetTime() * 10), glm::vec3(0.0f, 1.0f, 0.0f));

		// 1. translation: displace along circle with 'radius' in range [-offset, offset]
		float angle = (float)i / (float)amount * 360.0f;
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float x = sin(angle) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;
		model = glm::translate(model, glm::vec3(x, y, z));

		// 2. scale: scale between 0.01 and 0.04f
		float scale = (rand() % 100) / 1000.0f + 0.1;
		model = glm::scale(model, glm::vec3(scale));

		// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
		float rotAngle = (rand() % 360);
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));
		// 4. now add to list of matrices
		modelMatrices[i] = model;
	}

	for (unsigned int i = 0; i < amount; i++) {
		GLint modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
		glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelMatrices[i][0][0]);
		glDrawElements(GL_TRIANGLES, obj[4].indices.size(), GL_UNSIGNED_INT, 0);
	}
}


void paintGL(void)  //always run
{
	glClearColor(0.0f, 0.0f, 0.03f, 0.5f); //specify the background color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Depth test
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	UpdateStatus();

	//draw spacecraft model
	glBindVertexArray(vao[0]);
	drawObj(1);
	glDrawElements(GL_TRIANGLES, obj[0].indices.size(), GL_UNSIGNED_INT, 0);

	//draw planet model
	glBindVertexArray(vao[1]);
	drawObj(2);
	glDrawElements(GL_TRIANGLES, obj[1].indices.size(), GL_UNSIGNED_INT, 0);

	//draw alien v model 1-3
	for (int i = 6; i < 9; i++) {
		glBindVertexArray(vao[2]);
		drawObj(i);
		glDrawElements(GL_TRIANGLES, obj[2].indices.size(), GL_UNSIGNED_INT, 0);
	}

	//draw alien model 1-3
	for (int i = 9; i < 12; i++) {
		glBindVertexArray(vao[3]);
		drawObj(i);
		glDrawElements(GL_TRIANGLES, obj[3].indices.size(), GL_UNSIGNED_INT, 0);
	}

	//draw rock model
	glBindVertexArray(vao[4]);
	drawObj(3);
	Asteroids(amount);

	//draw chicken model
	glBindVertexArray(vao[5]);
	drawObj(4);
	glDrawElements(GL_TRIANGLES, obj[5].indices.size(), GL_UNSIGNED_INT, 0);

	//draw apple model 1-2
	glBindVertexArray(vao[6]);
	drawObj(5);
	glDrawElements(GL_TRIANGLES, obj[6].indices.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(vao[6]);
	drawObj(12);
	glDrawElements(GL_TRIANGLES, obj[6].indices.size(), GL_UNSIGNED_INT, 0);

	materialLight();
	pointLight();
	spotLight();
	directionalLight();

	ambientLight();
	specularLight();

}


// CONTROLS

struct MouseController {
	bool LEFT_BUTTON = false;
	bool RIGHT_BUTTON = false;
	double MOUSE_Clickx = 0.0, MOUSE_Clicky = 0.0;
	double MOUSE_X = 0.0, MOUSE_Y = 0.0;
	int click_time = glfwGetTime();
};
MouseController mouseCtl;

double collision_range = 2.0f;
double collision_range2 = 5.0f;
int leisure_trigger = 0;
bool trigger_detection[6] = { false };

bool firstMouse = true;
double lastX = 0.0;

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		firstMouse = false;
	}

	if (lastX > xpos)
	{
		theta += 3.0f;
		spaceship_Rot_M = glm::rotate(glm::mat4(1.0f), glm::radians(theta), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	if (lastX < xpos)
	{
		theta -= 3.0f;
		spaceship_Rot_M = glm::rotate(glm::mat4(1.0f), glm::radians(theta), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	lastX = xpos;
}

void cursor_position_callback(GLFWwindow* window, double x, double y)
{
	// Sets the cursor position callback for the current window

	if (mouseCtl.LEFT_BUTTON) {
		mouse_callback(window, x, y);
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// Sets the mouse-button callback for the current window.	
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		mouseCtl.LEFT_BUTTON = true;
	}
}

void winning_detection() {
	if (leisure_trigger >= 6)
		textureKey[0] = 1;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// Sets the scoll callback for the current window.
	zoom -= (float)yoffset;
	if (zoom < 1.0f)
		zoom = 1.0f;
	if (zoom > 45.0f)
		zoom = 45.0f;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// collision detection
	
	if ((SCInitialPos[0] + SCTranslation[0])< (chicken_location.x *0.01f + collision_range) 
		&& (SCInitialPos[0] + SCTranslation[0]) > (chicken_location.x * 0.01f - collision_range)
		&& (SCInitialPos[2] + SCTranslation[2]) < (chicken_location.z *0.01f + collision_range) 
		&& (SCInitialPos[0] + SCTranslation[2]) > (chicken_location.z * 0.01f - collision_range)
		&& trigger_detection[0] == false) {
		chicken_trigger = 0.0;
		leisure_trigger++;
		trigger_detection[0] = true;
		winning_detection();
	}

	if ((SCInitialPos[0] + SCTranslation[0]) < (apple_location_1.x + collision_range) && (SCInitialPos[0] + SCTranslation[0]) > (apple_location_1.x - collision_range)
		&& (SCInitialPos[2] + SCTranslation[2]) < (apple_location_1.z + collision_range) && (SCInitialPos[0] + SCTranslation[2]) > (apple_location_1.z - collision_range) 
		&& trigger_detection[1] == false) {
		apple_trigger_1 = 0.0;
		leisure_trigger++;
		trigger_detection[1] = true;
		winning_detection();
	}

	if ((SCInitialPos[0] + SCTranslation[0]) < (ufo_location_1.x * 0.5 + collision_range2) && (SCInitialPos[0] + SCTranslation[0]) > (ufo_location_1.x * 0.5 - collision_range2)
		&& (SCInitialPos[2] + SCTranslation[2]) < (ufo_location_1.z * 0.5 + collision_range2) && (SCInitialPos[0] + SCTranslation[2]) > (ufo_location_1.z * 0.5 - collision_range2)
		&& trigger_detection[2] == false) {
		ufo_textureKey[0] = 1;
		leisure_trigger++;
		trigger_detection[2] = true;
		winning_detection();
	}
	if ((SCInitialPos[0] + SCTranslation[0]) < (ufo_location_2.x * 0.5 + collision_range2) && (SCInitialPos[0] + SCTranslation[0]) > (ufo_location_2.x * 0.5 - collision_range2)
		&& (SCInitialPos[2] + SCTranslation[2]) < (ufo_location_2.z * 0.5 + collision_range2) && (SCInitialPos[0] + SCTranslation[2]) > (ufo_location_2.z * 0.5 - collision_range2)
		&& trigger_detection[3] == false) {
		ufo_textureKey[1] = 1;
		leisure_trigger++;
		trigger_detection[3] = true;
		winning_detection();
	}
	if ((SCInitialPos[0] + SCTranslation[0]) < (ufo_location_3.x * 0.5 + collision_range2) && (SCInitialPos[0] + SCTranslation[0]) > (ufo_location_3.x * 0.5 - collision_range2)
		&& (SCInitialPos[2] + SCTranslation[2]) < (ufo_location_3.z * 0.5 + collision_range2) && (SCInitialPos[0] + SCTranslation[2]) > (ufo_location_3.z * 0.5 - collision_range2)
		&& trigger_detection[4] == false) {
		ufo_textureKey[2] = 1;
		leisure_trigger++;
		trigger_detection[4] = true;
		winning_detection();
	}

	if ((SCInitialPos[0] + SCTranslation[0]) < (apple_location_2.x + collision_range) && (SCInitialPos[0] + SCTranslation[0]) > (apple_location_2.x - collision_range)
		&& (SCInitialPos[2] + SCTranslation[2]) < (apple_location_2.z + collision_range) && (SCInitialPos[0] + SCTranslation[2]) > (apple_location_2.z - collision_range)
		&& trigger_detection[5] == false) {
		apple_trigger_2 = 0.0;
		leisure_trigger++;
		trigger_detection[5] = true;
		winning_detection();
	}

	// Sets the Keyboard callback for the current window.
	
	//x movement sin(theta)* SC_world_Front_Direction[2] + sin(theta + 90.0) * SC_world_Right_Direction[0]
	//z movement cos(theta)* SC_world_Front_Direction[2] + cos(theta + 90.0) * SC_world_Right_Direction[0]
	//					when z moved										when x moved

	//spaceship movement
	if (key == GLFW_KEY_DOWN) {
		//moveZ -= 300;
		//SCTranslation[2] = SCTranslation[2] - SC_world_Front_Direction[2];
		SCTranslation[2] += cos(glm::radians(theta)) * -SC_world_Front_Direction[2];
		SCTranslation[0] += sin(glm::radians(theta)) * -SC_world_Front_Direction[2];
	}
	if (key == GLFW_KEY_UP) {
		//moveZ += 300;
		//SCTranslation[2] = SCTranslation[2] + SC_world_Front_Direction[2];
		SCTranslation[2] += cos(glm::radians(theta)) * SC_world_Front_Direction[2];
		SCTranslation[0] += sin(glm::radians(theta)) * SC_world_Front_Direction[2];
	}
	if (key == GLFW_KEY_LEFT) {
		//moveX += 300;
		//SCTranslation[0] = SCTranslation[0] + SC_world_Right_Direction[0];
		SCTranslation[2] += cos(glm::radians(theta) + 90) * SC_world_Front_Direction[0];
		SCTranslation[0] += sin(glm::radians(theta) + 90) * SC_world_Front_Direction[0];
	}
	if (key == GLFW_KEY_RIGHT) {
		//moveX -= 300;
		//SCTranslation[0] = SCTranslation[0] - SC_world_Right_Direction[0];
		SCTranslation[2] += cos(glm::radians(theta) + 90) * -SC_world_Front_Direction[0];
		SCTranslation[0] += sin(glm::radians(theta) + 90) * -SC_world_Front_Direction[0];
	}


	//light control
	if (key == GLFW_KEY_W && action == GLFW_PRESS && lightValue < 0.2) {
		lightValue += 0.2;
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS && lightValue > -0.2) {
		lightValue += -0.2;
	}


	if (key == GLFW_KEY_D && action == GLFW_PRESS && lightValue2 < 80.0) {
		lightValue2 += 10.0;
	}
	if (key == GLFW_KEY_A && action == GLFW_PRESS && lightValue2 > 10.0) {
		lightValue2 += -10.0;
	}

	if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
		if (lightValue4 == 0) {
			lightValue4 = 1;
		}
		else lightValue4 = 0;
	}
	if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		if (lightValue5 == 0) {
			lightValue5 = 1;
		}
		else lightValue5 = 0;
	}
}


int main(int argc, char* argv[])
{
	GLFWwindow* window;

	/* Initialize the glfw */
	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW" << std::endl;
		return -1;
	}
	/* glfw: configure; necessary for MAC */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3260 Project", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/*register callback functions*/
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	initializedGL();

	while (!glfwWindowShouldClose(window)) {
		/* Render here */
		paintGL();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}
