/*Práctica 6: Texturizado - Octaedro (Dado de 8 caras) Mapeo por Píxeles*/
#define STB_IMAGE_IMPLEMENTATION
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>
#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>
#include "Window.h"
#include "Mesh.h"
#include "Shader_m.h"
#include "Camera.h"
#include "Texture.h"
#include "Sphere.h"
#include"Model.h"
#include "Skybox.h"

const float toRadians = 3.14159265f / 180.0f;
Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;
Camera camera;

Texture pisoTexture;
Texture Dado_8caras;
Texture Textura_carro;

Model Kitt_M;
Model Llanta_M;
Skybox skybox;
Model Carro_texturizado_nuevo;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

static const char* vShader = "shaders/shader_texture.vert";
static const char* fShader = "shaders/shader_texture.frag";

// Función para calcular normales
void calcAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount,
	unsigned int vLength, unsigned int normalOffset) {
	for (size_t i = 0; i < indiceCount; i += 3) {
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);
		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}
	for (size_t i = 0; i < verticeCount / vLength; i++) {
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}

void CreateObjects() {
	unsigned int floorIndices[] = { 0, 2, 1, 1, 2, 3 };
	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};
	Mesh* obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);
}

void CrearDado8Caras() {
	// Formato: x, y, z,   u, v,   nx, ny, nz
	// Nota: He ajustado los valores UV para que coincidan con los puntos de tu imagen

	GLfloat oct_vertices[] = {
		// --- FILA SUPERIOR (Triángulos con punta hacia ARRIBA en la textura) ---

		// Cara 1 (1 punto) - Centro superior
		0.0f,  1.0f,  0.0f,  0.458f, 0.96f,   0.577f, 0.577f, 0.577f,
		1.0f,  0.0f,  0.0f,  0.556f, 0.53f,   0.577f, 0.577f, 0.577f,
		0.0f,  0.0f,  1.0f,  0.359f, 0.53f,   0.577f, 0.577f, 0.577f,

		// Cara 3 (3 puntos) - Derecha superior
		0.0f,  1.0f,  0.0f,  0.742f, 0.96f,   0.577f, 0.577f, -0.577f,
		0.0f,  0.0f, -1.0f,  0.840f, 0.53f,   0.577f, 0.577f, -0.577f,
		1.0f,  0.0f,  0.0f,  0.643f, 0.53f,   0.577f, 0.577f, -0.577f,

		// Cara 4 (4 puntos) - Extremo derecho superior
		0.0f,  1.0f,  0.0f,  0.855f, 0.35f,  -0.577f, 0.577f, -0.577f, // Reajustado por posición en net
	   -1.0f,  0.0f,  0.0f,  0.954f, 0.05f,  -0.577f, 0.577f, -0.577f,
		0.0f,  0.0f, -1.0f,  0.755f, 0.05f,  -0.577f, 0.577f, -0.577f,

		// Cara 7 (7 puntos) - Izquierda superior
		0.0f,  1.0f,  0.0f,  0.260f, 0.96f,  -0.577f, 0.577f, 0.577f,
		0.0f,  0.0f,  1.0f,  0.161f, 0.53f,  -0.577f, 0.577f, 0.577f,
	   -1.0f,  0.0f,  0.0f,  0.358f, 0.53f,  -0.577f, 0.577f, 0.577f,

	   // --- FILA INFERIOR (Triángulos con punta hacia ABAJO en la textura) ---

	   // Cara 2 (2 puntos)
	   0.0f, -1.0f,  0.0f,  0.855f, 0.12f,  -0.577f, -0.577f, -0.577f,
	   0.0f,  0.0f, -1.0f,  0.755f, 0.53f,  -0.577f, -0.577f, -0.577f,
	  -1.0f,  0.0f,  0.0f,  0.954f, 0.53f,  -0.577f, -0.577f, -0.577f,

	  // Cara 5 (5 puntos)
	  0.0f, -1.0f,  0.0f,  0.655f, 0.12f,   0.577f, -0.577f, -0.577f,
	  1.0f,  0.0f,  0.0f,  0.754f, 0.53f,   0.577f, -0.577f, -0.577f,
	  0.0f,  0.0f, -1.0f,  0.556f, 0.53f,   0.577f, -0.577f, -0.577f,

	  // Cara 6 (6 puntos)
	  0.0f, -1.0f,  0.0f,  0.458f, 0.12f,   0.577f, -0.577f, 0.577f,
	  0.0f,  0.0f,  1.0f,  0.359f, 0.53f,   0.577f, -0.577f, 0.577f,
	  1.0f,  0.0f,  0.0f,  0.556f, 0.53f,   0.577f, -0.577f, 0.577f,

	  // Cara 8 (8 puntos)
	  0.0f, -1.0f,  0.0f,  0.260f, 0.12f,  -0.577f, -0.577f, 0.577f,
	 -1.0f,  0.0f,  0.0f,  0.161f, 0.53f,  -0.577f, -0.577f, 0.577f,
	  0.0f,  0.0f,  1.0f,  0.359f, 0.53f,  -0.577f, -0.577f, 0.577f,
	};

	unsigned int oct_indices[] = {
		0, 1, 2,  3, 4, 5,  6, 7, 8,  9, 10, 11,
		12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23
	};

	Mesh* dado8 = new Mesh();
	dado8->CreateMesh(oct_vertices, oct_indices, 192, 24);
	meshList.push_back(dado8);
}

void CreateShaders() {
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

int main() {
	mainWindow = Window(1366, 768);
	mainWindow.Initialise();

	CreateObjects();
	CrearDado8Caras();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 2.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -20.0f, 10.0f, 0.8f);

	pisoTexture = Texture("Textures/piso.tga");
	pisoTexture.LoadTextureA();

	Dado_8caras = Texture("Textures/Dado_8caras.png"); // Asegúrate que el nombre coincida
	Dado_8caras.LoadTextureA();
	Textura_carro = Texture("Textures/Textura_carro.png");
	Textura_carro.LoadTextureA();
	Carro_texturizado_nuevo = Model();
	Carro_texturizado_nuevo.LoadModel("Models/Carro_texturizado_nuevo.obj");

	std::vector<std::string> skyboxFaces = {
		"Textures/Skybox/cupertin-lake_rt.tga", "Textures/Skybox/cupertin-lake_lf.tga",
		"Textures/Skybox/cupertin-lake_dn.tga", "Textures/Skybox/cupertin-lake_up.tga",
		"Textures/Skybox/cupertin-lake_bk.tga", "Textures/Skybox/cupertin-lake_ft.tga"
	};
	skybox = Skybox(skyboxFaces);

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	while (!mainWindow.getShouldClose()) {
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;

		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);

		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformColor = shaderList[0].getColorLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformColor, 1.0f, 1.0f, 1.0f);

		// Renderizar Piso
		glm::mat4 model(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		pisoTexture.UseTexture();
		meshList[0]->RenderMesh();

		// RENDERIZAR DADO DE 8 CARAS
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 1.0f, -2.0f));
		// Rotación suave para ver el mapeo
		model = glm::rotate(model, (float)0.5f, glm::vec3(0.0f, 1.0f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Dado_8caras.UseTexture();
		meshList[1]->RenderMesh();

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-3.0, 3.0, -2.0));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Carro_texturizado_nuevo.RenderModel();

		glUseProgram(0);
		mainWindow.swapBuffers();
	}
	return 0;
}