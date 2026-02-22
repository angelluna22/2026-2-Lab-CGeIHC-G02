#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <glew.h>
#include <glfw3.h>

const int WIDTH = 800, HEIGHT = 800;

GLuint VAO, VBO, shader;

float colorRojo = 0.0f;
float colorVerde = 0.0f;
float colorAzul = 0.0f;

double tiempoAnterior = 0.0;

// =======================
// VERTEX SHADER
// =======================
static const char* vShader = "                                 \n\
#version 330 core                                     \n\
layout (location = 0) in vec3 pos;                     \n\
void main()                                            \n\
{                                                      \n\
    gl_Position = vec4(pos, 1.0);                      \n\
}";

// =======================
// FRAGMENT SHADER
// =======================
static const char* fShader = "                         \n\
#version 330 core                                     \n\
out vec4 color;                                       \n\
void main()                                           \n\
{                                                     \n\
    color = vec4(1.0, 1.0, 1.0, 1.0);                 \n\
}";

// =======================
// FUNCION PARA CREAR LETRAS
// =======================
void CrearIniciales()
{
    GLfloat vertices[] = {

        // ================= J =================
        -0.85f,-0.2f,0,  -0.80f,-0.2f,0,  -0.80f,-0.7f,0,
        -0.85f,-0.2f,0,  -0.80f,-0.7f,0,  -0.85f,-0.7f,0,

        -0.95f,-0.7f,0,  -0.80f,-0.7f,0,  -0.80f,-0.75f,0,
        -0.95f,-0.7f,0,  -0.80f,-0.75f,0, -0.95f,-0.75f,0,


        // ================= A (forma triangular real) =================

        // Lado izquierdo inclinado
        -0.25f,-0.4f,0,   -0.20f,-0.4f,0,   -0.175f,0.1f,0,
        -0.20f,-0.4f,0,   -0.15f,0.1f,0,    -0.175f,0.1f,0,

        // Lado derecho inclinado
        -0.10f,-0.4f,0,   -0.05f,-0.4f,0,   -0.125f,0.1f,0,
        -0.05f,-0.4f,0,   -0.075f,0.1f,0,   -0.125f,0.1f,0,

        // Barra central
        -0.19f,-0.15f,0,  -0.11f,-0.15f,0,  -0.11f,-0.2f,0,
        -0.19f,-0.15f,0,  -0.11f,-0.2f,0,   -0.19f,-0.2f,0,


        // ================= V (más cerrada abajo) =================
        // Lado izquierdo
        0.2f,0.5f,0,  0.25f,0.5f,0,  0.32f,0.05f,0,
        0.2f,0.5f,0,  0.32f,0.05f,0,  0.27f,0.05f,0,

        // Lado derecho
        0.4f,0.5f,0,  0.45f,0.5f,0,  0.32f,0.05f,0,
        0.4f,0.5f,0,  0.32f,0.05f,0,  0.37f,0.05f,0,


        // ================= L =================
        0.6f,0.8f,0,  0.65f,0.8f,0,  0.65f,0.3f,0,
        0.6f,0.8f,0,  0.65f,0.3f,0,  0.6f,0.3f,0,

        0.6f,0.3f,0,  0.8f,0.3f,0,  0.8f,0.25f,0,
        0.6f,0.3f,0,  0.8f,0.25f,0, 0.6f,0.25f,0,
    };

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// =======================
// SHADERS
// =======================
void AddShader(GLuint program, const char* code, GLenum type)
{
    GLuint theShader = glCreateShader(type);
    glShaderSource(theShader, 1, &code, NULL);
    glCompileShader(theShader);
    glAttachShader(program, theShader);
}

void CompileShaders()
{
    shader = glCreateProgram();
    AddShader(shader, vShader, GL_VERTEX_SHADER);
    AddShader(shader, fShader, GL_FRAGMENT_SHADER);
    glLinkProgram(shader);
}

// =======================
// MAIN
// =======================
int main()
{
    srand(time(NULL));

    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "Iniciales JAVL", NULL, NULL);
    if (!mainWindow) { glfwTerminate(); return 1; }

    glfwMakeContextCurrent(mainWindow);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) return 1;

    glViewport(0, 0, WIDTH, HEIGHT);

    CrearIniciales();
    CompileShaders();

    tiempoAnterior = glfwGetTime();

    while (!glfwWindowShouldClose(mainWindow))
    {
        glfwPollEvents();

        double tiempoActual = glfwGetTime();

        if (tiempoActual - tiempoAnterior >= 2.0)
        {
            colorRojo = (float)rand() / RAND_MAX;
            colorVerde = (float)rand() / RAND_MAX;
            colorAzul = (float)rand() / RAND_MAX;
            tiempoAnterior = tiempoActual;
        }

        glClearColor(colorRojo, colorVerde, colorAzul, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 66);
        glBindVertexArray(0);

        glfwSwapBuffers(mainWindow);
    }

    glfwTerminate();
    return 0;
}