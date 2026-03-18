#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Mesh.h"
#include "Shader.h"
#include "Sphere.h"
#include "Window.h"
#include "Camera.h"

using std::vector;

const float PI = 3.14159265f;
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

Camera camera;
Window mainWindow;
vector<Mesh*> meshList;
vector<Shader> shaderList;
Sphere sp = Sphere(1.0, 20, 20);

// --- Funciones de creación de mallas ---

void CrearCubo() {
    unsigned int cubo_indices[] = {
        0, 1, 2, 2, 3, 0, 1, 5, 6, 6, 2, 1, 7, 6, 5, 5, 4, 7,
        4, 0, 3, 3, 7, 4, 4, 5, 1, 1, 0, 4, 3, 2, 6, 6, 7, 3
    };
    GLfloat cubo_vertices[] = {
        -0.5f, -0.5f,  0.5f, 0.5f, -0.5f,  0.5f, 0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f
    };
    Mesh* cubo = new Mesh();
    cubo->CreateMesh(cubo_vertices, cubo_indices, 24, 36);
    meshList.push_back(cubo); // meshList[0]
}

void CrearPrismaTriangular() {
    // Definimos un prisma cuya "punta" o arista superior esté en el origen local para facilitar la rotación
    unsigned int indices[] = {
        0, 1, 2,   // Cara frontal
        3, 4, 5,   // Cara trasera
        0, 3, 4, 0, 4, 1, // Lado derecho
        1, 4, 5, 1, 5, 2, // Base
        2, 5, 3, 2, 3, 0  // Lado izquierdo
    };
    GLfloat vertices[] = {
        0.0f,  0.5f, 0.0f,   // 0: Punta arriba frontal
        0.5f, -0.5f, 0.0f,   // 1: Base derecha frontal
       -0.5f, -0.5f, 0.0f,   // 2: Base izquierda frontal
        0.0f,  0.5f, -1.0f,  // 3: Punta arriba trasera
        0.5f, -0.5f, -1.0f,  // 4: Base derecha trasera
       -0.5f, -0.5f, -1.0f   // 5: Base izquierda trasera
    };
    Mesh* prisma = new Mesh();
    prisma->CreateMesh(vertices, indices, 18, 18);
    meshList.push_back(prisma); // meshList[1]
}

void CreateShaders() {
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles("shaders/shader.vert", "shaders/shader.frag");
    shaderList.push_back(*shader1);
}

int main() {
    mainWindow = Window(1280, 720);
    mainWindow.Initialise();

    CrearCubo();             // meshList[0]
    CrearPrismaTriangular(); // meshList[1]
    CreateShaders();

    camera = Camera(glm::vec3(0.0f, 2.0f, 12.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 5.0f, 0.5f);
    sp.init();
    sp.load();

    GLuint uniformModel = shaderList[0].getModelLocation();
    GLuint uniformProjection = shaderList[0].getProjectLocation();
    GLuint uniformView = shaderList[0].getViewLocation();
    GLuint uniformColor = shaderList[0].getColorLocation();
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);

    while (!mainWindow.getShouldClose()) {
        GLfloat now = (GLfloat)glfwGetTime();
        deltaTime = now - lastTime;
        lastTime = now;
        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderList[0].useShader();
        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

        // --- CUERPO ---
        glm::mat4 modelCuerpo = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f));
        glm::mat4 temp = glm::scale(modelCuerpo, glm::vec3(2.0f, 1.5f, 3.5f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(temp));
        glUniform3f(uniformColor, 0.9f, 0.9f, 0.9f); // Blanco
        meshList[0]->RenderMesh();

        glm::mat4 modelTrasero = glm::translate(modelCuerpo, glm::vec3(0.0f, -0.1f, -2.5f));
        temp = glm::scale(modelTrasero, glm::vec3(1.8f, 1.3f, 2.5f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(temp));
        glUniform3f(uniformColor, 0.6f, 0.6f, 0.6f); // Gris
        meshList[0]->RenderMesh();

        // --- CABEZA (Hija del Cuerpo) ---
        glm::mat4 modelCabeza = glm::translate(modelCuerpo, glm::vec3(0.0f, 0.8f, 2.0f));
        modelCabeza = glm::rotate(modelCabeza, glm::radians(mainWindow.getarticulacion1()), glm::vec3(1, 0, 0)); // F

        // Esfera Cuello
        temp = glm::scale(modelCabeza, glm::vec3(0.7f));
        glUniform3f(uniformColor, 1.0f, 1.0f, 1.0f);
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(temp));
        sp.render();

        // Cubo Cabeza
        glm::mat4 cuboCabezaPos = glm::translate(modelCabeza, glm::vec3(0, 0.5f, 0.5f));
        temp = glm::scale(cuboCabezaPos, glm::vec3(1.3f, 1.3f, 1.3f));
        glUniform3f(uniformColor, 0.9f, 0.9f, 0.9f);
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(temp));
        meshList[0]->RenderMesh();

        // Rostro (Hijo de Cabeza)
        temp = glm::translate(cuboCabezaPos, glm::vec3(0.0f, -0.3f, 0.8f)); // Hocico
        temp = glm::scale(temp, glm::vec3(0.8f, 0.6f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(temp));
        meshList[0]->RenderMesh();

        temp = glm::translate(cuboCabezaPos, glm::vec3(0.0f, -0.2f, 1.3f)); // Nariz
        temp = glm::scale(temp, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniform3f(uniformColor, 0.0f, 0.0f, 0.0f);
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(temp));
        meshList[0]->RenderMesh();

        // Ojos (Esferas)
        temp = glm::translate(cuboCabezaPos, glm::vec3(0.35f, 0.3f, 0.6f));
        temp = glm::scale(temp, glm::vec3(0.1f, 0.2f, 0.1f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(temp));
        sp.render();
        temp = glm::translate(cuboCabezaPos, glm::vec3(-0.35f, 0.3f, 0.6f));
        temp = glm::scale(temp, glm::vec3(0.1f, 0.2f, 0.1f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(temp));
        sp.render();

        // OREJAS (Cubos Grises) - Tecla G
        glUniform3f(uniformColor, 0.5f, 0.5f, 0.5f);
        for (int i : {-1, 1}) {
            glm::mat4 modelOreja = glm::translate(cuboCabezaPos, glm::vec3(0.45f * i, 0.7f, 0.0f));
            modelOreja = glm::rotate(modelOreja, glm::radians(mainWindow.getarticulacion2() * i), glm::vec3(0, 0, 1));
            temp = glm::scale(modelOreja, glm::vec3(0.4f, 0.5f, 0.2f));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(temp));
            meshList[0]->RenderMesh();
        }

        // --- PATAS (4 mallas: Delanteras H, Traseras J) ---
        float posX[4] = { 0.8f, -0.8f, 0.7f, -0.7f };
        float posZ[4] = { 1.2f, 1.2f, -1.0f, -1.0f };
        for (int i = 0; i < 4; i++) {
            glm::mat4 basePata = (i < 2) ? modelCuerpo : modelTrasero;
            glm::mat4 modelPata = glm::translate(basePata, glm::vec3(posX[i], -0.8f, posZ[i]));
            float rot = (i < 2) ? mainWindow.getarticulacion3() : mainWindow.getarticulacion4();
            modelPata = glm::rotate(modelPata, glm::radians(rot), glm::vec3(1, 0, 0));

            temp = glm::scale(modelPata, glm::vec3(0.4f)); // Esfera
            glUniform3f(uniformColor, 1, 1, 1);
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(temp));
            sp.render();

            temp = glm::translate(modelPata, glm::vec3(0, -0.6f, 0)); // Pierna
            temp = glm::scale(temp, glm::vec3(0.5f, 1.2f, 0.5f));
            glUniform3f(uniformColor, 0.7f, 0.7f, 0.7f);
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(temp));
            meshList[0]->RenderMesh();

            temp = glm::translate(modelPata, glm::vec3(0, -1.3f, 0.1f)); // Pie
            temp = glm::scale(temp, glm::vec3(0.6f, 0.3f, 0.7f));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(temp));
            meshList[0]->RenderMesh();
        }

        // --- COLA (Prisma Triangular) - Tecla K ---
        glm::mat4 modelCola = glm::translate(modelTrasero, glm::vec3(0.0f, 0.5f, -1.3f));
        modelCola = glm::rotate(modelCola, glm::radians(mainWindow.getarticulacion5()), glm::vec3(0, 1, 0)); // Rotación lateral
        modelCola = glm::rotate(modelCola, glm::radians(-30.0f), glm::vec3(1, 0, 0)); // Inclinación base

        // Esfera unión
        temp = glm::scale(modelCola, glm::vec3(0.3f));
        glUniform3f(uniformColor, 1, 1, 1);
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(temp));
        sp.render();

        // Prisma (La punta está en el origen del modelo creado en CrearPrismaTriangular)
        temp = glm::scale(modelCola, glm::vec3(0.8f, 0.8f, 2.5f));
        glUniform3f(uniformColor, 0.5f, 0.5f, 0.5f);
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(temp));
        meshList[1]->RenderMesh();

        glUseProgram(0);
        mainWindow.swapBuffers();
    }
    return 0;
}