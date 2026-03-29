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
#include <assimp/Importer.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_m.h"
#include "Camera.h"
#include "Sphere.h"
#include "Model.h"
#include "Skybox.h"

const float toRadians = 3.14159265f / 180.0f;
Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;
Camera camera;

// Modelos de las piezas separadas
Model Carroceria, Cofre, ID, DD, TI, TD;
Skybox skybox;

// Variables de ángulo para cada pieza independiente
float movCoche = 0.0f;
float rotLlantas = 0.0f;
float rotCofre = 0.0f;

const float LIMITE = 45.0f;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

static const char* vShader = "shaders/shader_m.vert";
static const char* fShader = "shaders/shader_m.frag";

void CreateObjects() {
    unsigned int floorIndices[] = { 0, 2, 1, 1, 2, 3 };
    GLfloat floorVertices[] = {
        -10.0f, 0.0f, -10.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        10.0f, 0.0f, -10.0f, 10.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        -10.0f, 0.0f, 10.0f, 0.0f, 10.0f, 0.0f, -1.0f, 0.0f,
        10.0f, 0.0f, 10.0f, 10.0f, 10.0f, 0.0f, -1.0f, 0.0f
    };
    Mesh* objPiso = new Mesh();
    objPiso->CreateMesh(floorVertices, floorIndices, 32, 6);
    meshList.push_back(objPiso);
}

void CreateShaders() {
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

int main() {
    mainWindow = Window(1366, 768);
    mainWindow.Initialise();
    glEnable(GL_DEPTH_TEST);

    CreateObjects();
    CreateShaders();

    camera = Camera(glm::vec3(0.0f, 0.5f, 7.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 1.0f);

    // Cargar cada archivo .obj por separado
    Carroceria = Model();
    Carroceria.LoadModel("Models/Carroceria.obj");
    Cofre = Model();
    Cofre.LoadModel("Models/Cofre.obj");
    ID = Model();
    ID.LoadModel("Models/ID.obj");
    DD = Model();
    DD.LoadModel("Models/DD.obj");
    TD = Model();
    TD.LoadModel("Models/TD.obj");
    TI = Model();
    TI.LoadModel("Models/TI.obj");

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

        // --- LÓGICA DE LAS 8 TECLAS ---

        if (mainWindow.getsKeys()[GLFW_KEY_W]) {
            movCoche += 0.1f;
            rotLlantas += 5.0f;
        }

        if (mainWindow.getsKeys()[GLFW_KEY_S]) {
            movCoche -= 0.1f;
            rotLlantas -= 5.0f;
        }

        if (mainWindow.getsKeys()[GLFW_KEY_O]) rotCofre += 2.0f;
        if (mainWindow.getsKeys()[GLFW_KEY_P]) rotCofre -= 2.0f;

        // Aplicar límites de 45° a todas las variables
        rotCofre = glm::clamp(rotCofre, 0.0f,-45.0f);

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

        // --- RENDERIZADO DEL PISO ---
        glm::mat4 model = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, -2.0f, 0.0f));
        model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(uniformColor, 0.5f, 0.5f, 0.5f);
        meshList[0]->RenderMesh();

        // --- MATRIZ MAESTRA (CARROCERÍA) ---
        glm::mat4 modelCarro = glm::mat4(1.0f);
        // Usamos movCoche aquí para que todo el grupo se mueva
        modelCarro = glm::translate(modelCarro, glm::vec3(0.0f, -1.0f, movCoche));
        glUniform3f(uniformColor, 0.2f, 0.2f, 0.2f);
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelCarro));
        Carroceria.RenderModel();

        // --- LLANTA DELANTERA DERECHA (DD) ---
        // Blender: X: 1.353, Y: -1.251, Z: 0.169
        glm::mat4 mDD = modelCarro;
        mDD = glm::translate(mDD, glm::vec3(1.353f, 0.169f,-1.251f)); // Mapeo: X, Z, Y
        mDD = glm::rotate(mDD, glm::radians(rotLlantas), glm::vec3(1, 0, 0)); // Prueba con (1,0,0) o (0,0,1)
        glUniform3f(uniformColor, 0.0f, 0.0f, 1.0f); // Azul
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(mDD));
        DD.RenderModel();

        // --- LLANTA DELANTERA IZQUIERDA (ID) ---
        // Blender: X: -1.226, Y: -1.278, Z: 0.147
        glm::mat4 mID = modelCarro;
        mID = glm::translate(mID, glm::vec3(-1.226f, 0.147f, -1.278f));
        mID = glm::rotate(mID, glm::radians(rotLlantas), glm::vec3(1, 0, 0));
        glUniform3f(uniformColor, 0.0f, 1.0f, 0.0f); // Verde
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(mID));
        ID.RenderModel();

        // --- LLANTA TRASERA DERECHA (TD) ---
        // Blender: X: 1.268, Y: 1.188, Z: 0.134
        glm::mat4 mTD = modelCarro;
        mTD = glm::translate(mTD, glm::vec3(1.268f, 0.134f,1.188f));
        mTD = glm::rotate(mTD, glm::radians(rotLlantas), glm::vec3(1, 0, 0));
        glUniform3f(uniformColor, 1.0f, 0.0f, 0.0f); // Rojo
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(mTD));
        TD.RenderModel();

        // --- LLANTA TRASERA IZQUIERDA (TI) ---
        // Blender: X: -1.395, Y: 1.189, Z: 0.140
        glm::mat4 mTI = modelCarro;
        mTI = glm::translate(mTI, glm::vec3(-1.395f, 0.140f,1.189f));
        mTI = glm::rotate(mTI, glm::radians(rotLlantas), glm::vec3(1, 0, 0));
        glUniform3f(uniformColor, 1.0f, 1.0f, 0.0f); // Amarillo
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(mTI));
        TI.RenderModel();

        // --- COFRE ---
        // Blender: X: 0.202, Y: -1.220, Z: 1.289
        glm::mat4 mCofre = modelCarro;
        mCofre = glm::translate(mCofre, glm::vec3(0.202f, 1.289f, 0.5f));
        mCofre = glm::rotate(mCofre, glm::radians(rotCofre), glm::vec3(1, 0, 0));
        glUniform3f(uniformColor, 1.0f, 0.0f, 1.0f); // Magenta
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(mCofre));
        Cofre.RenderModel();

        glUseProgram(0);
        mainWindow.swapBuffers();
    }
    return 0;
}