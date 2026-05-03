/*
Animacion:
- Humo con blending, textura animada, crece/decrece segun el fuego
- Brazo/palanca activado por detector cerca del Aeolipile
- Esfera metalica con fisica basica: tiro parabolico y rebote
- Hueco en el piso + canal subterraneo que regresa la esfera al brazo
*/

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
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Sphere.h"
#include"Model.h"
#include "Skybox.h"

#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"

const float toRadians = 3.14159265f / 180.0f;

// =============================================
// VARIABLES ORIGINALES
// =============================================
float movCoche;
float movOffset;
float rotllanta;
float rotllantaOffset;
bool  avanza;
float toffsetflechau    = 0.0f;
float toffsetflechav    = 0.0f;
float toffsetnumerou    = 0.0f;
float toffsetnumerov    = 0.0f;
float toffsetnumerocambiau = 0.0f;
float angulovaria       = 0.0f;
bool  usaNumero         = 1;

float rotaAelo  = 0.0f;
float velAelo   = 90.0f;

// Fuego (sprite sheet)
bool  fuegoActivo    = false;
float tiempoFuego    = 0.0f;
float intervaloFuego = 0.08f;
int   frameFuegoU    = 0;
int   frameFuegoV    = 0;
float toffsetFuegoU  = 0.0f;
float toffsetFuegoV  = 0.0f;

// Numero cambiante
float timerNumeroCambia  = 0.0f;
float intervalNumeroCambia = 7.0f;

bool teclaFPresionada = false;

// =============================================
// NUEVAS VARIABLES: HUMO
// =============================================
// El humo tiene su propio sprite sheet (Humo.png)
// Crece cuando el fuego lleva tiempo activo, decrece cuando se apaga
float humoEscala          = 0.0f;   // escala actual del humo (0 = invisible)
float humoEscalaMax       = 3.0f;   // escala maxima
float humoVelCrecimiento  = 0.8f;   // unidades/seg creciendo
float humoVelDecrecimiento = 0.5f;  // unidades/seg decreciendo
bool  humoVisible         = false;
// Animacion sprite humo
float tiempoHumo    = 0.0f;
float intervaloHumo = 0.12f;
int   frameHumoU    = 0;
int   frameHumoV    = 0;
float toffsetHumoU  = 0.0f;
float toffsetHumoV  = 0.0f;
// Tiempo que tarda el Aeolipile en "calentarse" antes de salir humo
float tiempoCalentamiento    = 0.0f;
float calentamientoRequerido = 2.0f; // 2 segundos de fuego para que salga humo

// =============================================
// NUEVAS VARIABLES: BRAZO / PALANCA
// =============================================
// Detector: si rotaAelo supera cierto umbral el brazo se activa
float umbralDetectorBrazo = 180.0f; // grados acumulados para activar brazo
bool  brazolActivado      = false;
bool  brazoDisparado      = false;  // ya lanzo la esfera

// Angulo del brazo: parte en 0, sube hasta 90 (cargado), luego baja rapido (lanzamiento)
float anguloBrazo         = 0.0f;
float velCargaBrazo       = 30.0f;  // grados/seg subiendo
float velLanzamientoBrazo = 200.0f; // grados/seg bajando (rapido al lanzar)
bool  brazoCargando       = false;
bool  brazoLanzando       = false;
bool  brazoEnReposo       = true;
float anguloBrazoMax      = 85.0f;  // angulo maximo de carga

// =============================================
// NUEVAS VARIABLES: ESFERA METALICA
// =============================================
// Posicion inicial: encima del brazo (cerca del Aeolipile)
glm::vec3 posEsfera          = glm::vec3(7.8f, -0.2f, 1.5f); // posicion actual
glm::vec3 posInicialEsfera   = glm::vec3(7.8f, -0.2f, 1.5f); // descansa en la punta del brazo recostado
bool      esferaEnBrazo      = true;
bool      esferaEnVuelo      = false;
bool      esferaEnCanal      = false;    // viajando bajo el piso de regreso

// Fisica: tiro parabolico
glm::vec3 velEsfera    = glm::vec3(0.0f, 0.0f, 0.0f);
float     gravedad     = -9.8f;
int       numRebotes   = 0;
int       maxRebotes   = 3;
float     coefRebote   = 0.55f;  // cuanto rebota (0=nada, 1=perfectamente elastico)
float     radioEsfera  = 0.3f;
float     pisoPosY     = -2.0f;  // altura del piso

// Posicion del hueco en el piso (donde entra la esfera)
glm::vec3 posHueco     = glm::vec3(8.0f, pisoPosY, 1.5f);
float     radioHueco   = 0.8f;   // radio de deteccion del hueco

// Canal subterraneo: la esfera viaja de posHueco a posInicialEsfera bajo el piso
float     velocidadCanal = 6.0f; // velocidad de avance en el canal
float     canalPosY      = -4.5f; // Y del canal (bajo el piso)

// =============================================
// OPENGL / SCENE
// =============================================
Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;
Camera camera;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;
Texture pisoTexture;
Texture AgaveTexture;
Texture FlechaTexture;
Texture NumerosTexture;
Texture Numero1Texture;
Texture Numero2Texture;
Texture FuegoTex;
Texture HumoTex;      // NUEVO: textura humo

Model Kitt_M;
Model Llanta_M;
Model Pista_M;
Model Nave_M;
Model Ala_M;
Model Aeolipile_base_M;
Model Aeolipile_M;

Skybox skybox;

Material Material_brillante;
Material Material_opaco;
Material Material_metal;   // NUEVO: material metalico para la esfera

GLfloat deltaTime = 0.0f;
GLfloat lastTime  = 0.0f;
static double limitFPS = 1.0 / 60.0;

DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight  spotLights[MAX_SPOT_LIGHTS];

static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

// =============================================
// CALCULAR NORMALES
// =============================================
void calcAverageNormals(unsigned int* indices, unsigned int indiceCount,
    GLfloat* vertices, unsigned int verticeCount,
    unsigned int vLength, unsigned int normalOffset)
{
    for (size_t i = 0; i < indiceCount; i += 3)
    {
        unsigned int in0 = indices[i]     * vLength;
        unsigned int in1 = indices[i + 1] * vLength;
        unsigned int in2 = indices[i + 2] * vLength;
        glm::vec3 v1(vertices[in1]-vertices[in0], vertices[in1+1]-vertices[in0+1], vertices[in1+2]-vertices[in0+2]);
        glm::vec3 v2(vertices[in2]-vertices[in0], vertices[in2+1]-vertices[in0+1], vertices[in2+2]-vertices[in0+2]);
        glm::vec3 normal = glm::normalize(glm::cross(v1, v2));

        in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
        vertices[in0]   += normal.x; vertices[in0+1] += normal.y; vertices[in0+2] += normal.z;
        vertices[in1]   += normal.x; vertices[in1+1] += normal.y; vertices[in1+2] += normal.z;
        vertices[in2]   += normal.x; vertices[in2+1] += normal.y; vertices[in2+2] += normal.z;
    }
    for (size_t i = 0; i < verticeCount / vLength; i++)
    {
        unsigned int nOffset = i * vLength + normalOffset;
        glm::vec3 vec(vertices[nOffset], vertices[nOffset+1], vertices[nOffset+2]);
        vec = glm::normalize(vec);
        vertices[nOffset] = vec.x; vertices[nOffset+1] = vec.y; vertices[nOffset+2] = vec.z;
    }
}

// =============================================
// CREATE OBJECTS
// =============================================
void CreateObjects()
{
    // --- Piramide ---
    unsigned int indices[] = { 0,3,1, 1,3,2, 2,3,0, 0,1,2 };
    GLfloat vertices[] = {
        -1.0f,-1.0f,-0.6f, 0.0f,0.0f, 0.0f,0.0f,0.0f,
         0.0f,-1.0f, 1.0f, 0.5f,0.0f, 0.0f,0.0f,0.0f,
         1.0f,-1.0f,-0.6f, 1.0f,0.0f, 0.0f,0.0f,0.0f,
         0.0f, 1.0f, 0.0f, 0.5f,1.0f, 0.0f,0.0f,0.0f
    };

    // --- Piso ---
    unsigned int floorIndices[] = { 0,2,1, 1,2,3 };
    GLfloat floorVertices[] = {
        -10.0f,0.0f,-10.0f, 0.0f, 0.0f, 0.0f,-1.0f,0.0f,
         10.0f,0.0f,-10.0f,10.0f, 0.0f, 0.0f,-1.0f,0.0f,
        -10.0f,0.0f, 10.0f, 0.0f,10.0f, 0.0f,-1.0f,0.0f,
         10.0f,0.0f, 10.0f,10.0f,10.0f, 0.0f,-1.0f,0.0f
    };

    // --- Vegetacion (cruz) ---
    unsigned int vegetacionIndices[] = { 0,1,2, 0,2,3, 4,5,6, 4,6,7 };
    GLfloat vegetacionVertices[] = {
        -0.5f,-0.5f,0.0f, 0.0f,0.0f, 0.0f,0.0f,0.0f,
         0.5f,-0.5f,0.0f, 1.0f,0.0f, 0.0f,0.0f,0.0f,
         0.5f, 0.5f,0.0f, 1.0f,1.0f, 0.0f,0.0f,0.0f,
        -0.5f, 0.5f,0.0f, 0.0f,1.0f, 0.0f,0.0f,0.0f,
         0.0f,-0.5f,-0.5f,0.0f,0.0f, 0.0f,0.0f,0.0f,
         0.0f,-0.5f, 0.5f,1.0f,0.0f, 0.0f,0.0f,0.0f,
         0.0f, 0.5f, 0.5f,1.0f,1.0f, 0.0f,0.0f,0.0f,
         0.0f, 0.5f,-0.5f,0.0f,1.0f, 0.0f,0.0f,0.0f
    };

    // --- Flecha ---
    unsigned int flechaIndices[] = { 0,1,2, 0,2,3 };
    GLfloat flechaVertices[] = {
        -0.5f,0.0f, 0.5f, 0.0f,0.0f, 0.0f,-1.0f,0.0f,
         0.5f,0.0f, 0.5f, 1.0f,0.0f, 0.0f,-1.0f,0.0f,
         0.5f,0.0f,-0.5f, 1.0f,1.0f, 0.0f,-1.0f,0.0f,
        -0.5f,0.0f,-0.5f, 0.0f,1.0f, 0.0f,-1.0f,0.0f
    };

    // --- Score (todos los numeros) ---
    unsigned int scoreIndices[] = { 0,1,2, 0,2,3 };
    GLfloat scoreVertices[] = {
        -0.5f,0.0f, 0.5f, 0.0f,0.0f, 0.0f,-1.0f,0.0f,
         0.5f,0.0f, 0.5f, 1.0f,0.0f, 0.0f,-1.0f,0.0f,
         0.5f,0.0f,-0.5f, 1.0f,1.0f, 0.0f,-1.0f,0.0f,
        -0.5f,0.0f,-0.5f, 0.0f,1.0f, 0.0f,-1.0f,0.0f
    };

    // --- Un solo numero ---
    unsigned int numeroIndices[] = { 0,1,2, 0,2,3 };
    GLfloat numeroVertices[] = {
        -0.5f,0.0f, 0.5f, 0.0f, 0.67f, 0.0f,-1.0f,0.0f,
         0.5f,0.0f, 0.5f, 0.25f,0.67f, 0.0f,-1.0f,0.0f,
         0.5f,0.0f,-0.5f, 0.25f,1.0f,  0.0f,-1.0f,0.0f,
        -0.5f,0.0f,-0.5f, 0.0f, 1.0f,  0.0f,-1.0f,0.0f
    };

    // -----------------------------------------------
    // NUEVO: Quad para humo (billboard vertical)
    // meshList[7]
    // -----------------------------------------------
    unsigned int humoIndices[] = { 0,1,2, 0,2,3 };
    GLfloat humoVertices[] = {
        -0.5f,-0.5f,0.0f, 0.0f,0.0f, 0.0f,0.0f,1.0f,
         0.5f,-0.5f,0.0f, 1.0f,0.0f, 0.0f,0.0f,1.0f,
         0.5f, 0.5f,0.0f, 1.0f,1.0f, 0.0f,0.0f,1.0f,
        -0.5f, 0.5f,0.0f, 0.0f,1.0f, 0.0f,0.0f,1.0f
    };

    // -----------------------------------------------
    // NUEVO: Quad para brazo (rectangulo alargado)
    // meshList[8]
    // El pivote esta en el extremo inferior (y=-0.5 -> y=+0.5 es la longitud)
    // -----------------------------------------------
    unsigned int brazoIndices[] = {
        0,1,2, 0,2,3,  // cara frontal
        4,5,6, 4,6,7   // cara trasera
    };
    GLfloat brazoVertices[] = {
        // cara frontal
        -0.15f, 0.0f, 0.0f,  0.0f,0.0f,  0.0f,0.0f,1.0f,
         0.15f, 0.0f, 0.0f,  1.0f,0.0f,  0.0f,0.0f,1.0f,
         0.15f, 2.0f, 0.0f,  1.0f,1.0f,  0.0f,0.0f,1.0f,
        -0.15f, 2.0f, 0.0f,  0.0f,1.0f,  0.0f,0.0f,1.0f,
        // cara trasera
        -0.15f, 0.0f,-0.1f,  0.0f,0.0f,  0.0f,0.0f,-1.0f,
         0.15f, 0.0f,-0.1f,  1.0f,0.0f,  0.0f,0.0f,-1.0f,
         0.15f, 2.0f,-0.1f,  1.0f,1.0f,  0.0f,0.0f,-1.0f,
        -0.15f, 2.0f,-0.1f,  0.0f,1.0f,  0.0f,0.0f,-1.0f
    };

    // -----------------------------------------------
    // NUEVO: Esfera (quad con textura, se mueve por codigo)
    // meshList[9]  -- quad simple para representar la esfera visualmente
    // -----------------------------------------------
    unsigned int esferaIndices[] = { 0,1,2, 0,2,3 };
    GLfloat esferaVertices[] = {
        -0.3f,-0.3f,0.0f, 0.0f,0.0f, 0.0f,0.0f,1.0f,
         0.3f,-0.3f,0.0f, 1.0f,0.0f, 0.0f,0.0f,1.0f,
         0.3f, 0.3f,0.0f, 1.0f,1.0f, 0.0f,0.0f,1.0f,
        -0.3f, 0.3f,0.0f, 0.0f,1.0f, 0.0f,0.0f,1.0f
    };

    // -----------------------------------------------
    // Agregar todos los meshes
    // -----------------------------------------------
    Mesh* obj1 = new Mesh(); obj1->CreateMesh(vertices, indices, 32, 12);
    meshList.push_back(obj1);  // [0]

    Mesh* obj2 = new Mesh(); obj2->CreateMesh(vertices, indices, 32, 12);
    meshList.push_back(obj2);  // [1]

    Mesh* obj3 = new Mesh(); obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
    meshList.push_back(obj3);  // [2] piso

    Mesh* obj4 = new Mesh(); obj4->CreateMesh(vegetacionVertices, vegetacionIndices, 64, 12);
    meshList.push_back(obj4);  // [3]

    Mesh* obj5 = new Mesh(); obj5->CreateMesh(flechaVertices, flechaIndices, 32, 6);
    meshList.push_back(obj5);  // [4] flecha/fuego

    Mesh* obj6 = new Mesh(); obj6->CreateMesh(scoreVertices, scoreIndices, 32, 6);
    meshList.push_back(obj6);  // [5] score completo

    Mesh* obj7 = new Mesh(); obj7->CreateMesh(numeroVertices, numeroIndices, 32, 6);
    meshList.push_back(obj7);  // [6] un solo numero

    Mesh* obj8 = new Mesh(); obj8->CreateMesh(humoVertices, humoIndices, 32, 6);
    meshList.push_back(obj8);  // [7] quad humo

    Mesh* obj9 = new Mesh(); obj9->CreateMesh(brazoVertices, brazoIndices, 64, 12);
    meshList.push_back(obj9);  // [8] brazo

    Mesh* obj10 = new Mesh(); obj10->CreateMesh(esferaVertices, esferaIndices, 32, 6);
    meshList.push_back(obj10); // [9] esfera
}

void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

// =============================================
// LOGICA DE FISICA DE LA ESFERA
// =============================================
void ActualizarEsfera(float dt)
{
    if (esferaEnBrazo) return; // la esfera espera en el brazo, sin fisica

    if (esferaEnVuelo)
    {
        // Aplicar gravedad
        velEsfera.y += gravedad * dt;

        // Mover la esfera
        posEsfera += velEsfera * dt;

        // Colision con el piso
        if (posEsfera.y <= pisoPosY + radioEsfera)
        {
            posEsfera.y = pisoPosY + radioEsfera;

            // Revisar si cae dentro del hueco
            float distHueco = glm::length(glm::vec2(posEsfera.x - posHueco.x,
                                                     posEsfera.z - posHueco.z));
            if (distHueco < radioHueco)
            {
                // Entra al hueco -> va al canal subterraneo
                esferaEnVuelo = false;
                esferaEnCanal = true;
                posEsfera.y   = canalPosY;
                velEsfera     = glm::vec3(0.0f); // resetear velocidad
                printf("[ESFERA] Entro al hueco! Iniciando canal subterraneo.\n");
            }
            else
            {
                // Rebota
                numRebotes++;
                velEsfera.y = -velEsfera.y * coefRebote;
                velEsfera.x *= 0.85f;
                velEsfera.z *= 0.85f;

                // Si ya reboto suficiente: mandar directo al canal, sin mas rebotes
                if (numRebotes >= maxRebotes || fabsf(velEsfera.y) < 0.5f)
                {
                    esferaEnVuelo = false;
                    esferaEnCanal = true;
                    posEsfera.y = canalPosY;
                    velEsfera = glm::vec3(0.0f);
                    printf("[ESFERA] Ultimo rebote, regresando al brazo.\n");
                }
            }
        }
    }
    else if (esferaEnCanal)
    {
        // Mover suavemente hacia la posicion inicial del brazo (por debajo del piso)
        glm::vec3 destino = glm::vec3(posInicialEsfera.x, canalPosY, posInicialEsfera.z);
        glm::vec3 dir = destino - posEsfera;
        float dist = glm::length(dir);

        if (dist < 0.1f)
        {
            // Llego al final del canal: subir de regreso al brazo
            posEsfera     = posInicialEsfera;
            esferaEnCanal = false;
            esferaEnBrazo = true;
            numRebotes    = 0;

            // Resetear brazo para poder lanzar de nuevo
            brazoDisparado  = false;
            brazoLanzando   = false;
            brazoCargando   = false;
            brazoEnReposo   = true;
            brazolActivado  = false;
            anguloBrazo     = 0.0f;
            rotaAelo        = 0.0f; 
            velAelo = 90.0f;// resetear giros del Aeolipile para nuevo ciclo
            printf("[ESFERA] Regreso al brazo. Lista para un nuevo lanzamiento.\n");
        }
        else
        {
            posEsfera += (dir / dist) * velocidadCanal * dt;
        }
    }
}

// =============================================
// LOGICA DEL BRAZO / DETECTOR
// =============================================
void ActualizarBrazo(float dt)
{
    // Solo activa si el sensor detecto suficiente giro Y la esfera esta lista
    if (!brazolActivado && !brazoDisparado && fuegoActivo && esferaEnBrazo)
    {
        if (rotaAelo >= umbralDetectorBrazo)
        {
            brazolActivado = true;
            brazoLanzando = true;   // va directo al lanzamiento, sin carga
            brazoEnReposo = false;
            anguloBrazo = anguloBrazoMax; // empieza desde el angulo maximo (ya "cargado")
            printf("[BRAZO] Sensor activado! Lanzando.\n");
        }
    }

    // LANZAMIENTO: baja rapido desde anguloBrazoMax hasta 0
    if (brazoLanzando)
    {
        anguloBrazo -= velLanzamientoBrazo * dt;
        if (anguloBrazo <= 0.0f)
        {
            anguloBrazo = 0.0f;
            brazoLanzando = false;
            brazoDisparado = true;
            brazoEnReposo = true;

            if (esferaEnBrazo)
            {
                esferaEnBrazo = false;
                esferaEnVuelo = true;
                posEsfera = posInicialEsfera;
                velEsfera = glm::vec3(3.0f, 12.0f, 0.0f);
                printf("[ESFERA] Lanzada!\n");
            }
        }
    }
}

// =============================================
// LOGICA DEL HUMO
// =============================================
void ActualizarHumo(float dt)
{
    if (fuegoActivo)
    {
        // Calentamiento
        tiempoCalentamiento += dt;

        // Solo empieza a crecer cuando hay suficiente calentamiento
        if (tiempoCalentamiento >= calentamientoRequerido)
        {
            humoVisible = true;
            humoEscala  = fminf(humoEscala + humoVelCrecimiento * dt, humoEscalaMax);
        }

        // Animacion sprite del humo
        tiempoHumo += dt;
        if (tiempoHumo >= intervaloHumo)
        {
            tiempoHumo = 0.0f;
            frameHumoU++;
            if (frameHumoU >= 4) {
                frameHumoU = 0;
                frameHumoV++;
                if (frameHumoV >= 4) frameHumoV = 0;
            }
        }
        toffsetHumoU = frameHumoU * 0.25f;
        toffsetHumoV = frameHumoV * 0.25f;
    }
    else
    {
        // Fuego apagado: el humo decrece gradualmente
        tiempoCalentamiento = 0.0f;
        humoEscala -= humoVelDecrecimiento * dt;
        if (humoEscala <= 0.0f)
        {
            humoEscala  = 0.0f;
            humoVisible = false;
        }
    }
}

// =============================================
// MAIN
// =============================================
int main()
{
    mainWindow = Window(1366, 768);
    mainWindow.Initialise();

    CreateObjects();
    CreateShaders();

    camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,1.0f,0.0f),
                    -60.0f, 0.0f, 5.0f, 0.5f);

    // Texturas originales
    brickTexture  = Texture("Textures/brick.png");      brickTexture.LoadTextureA();
    dirtTexture   = Texture("Textures/dirt.png");       dirtTexture.LoadTextureA();
    plainTexture  = Texture("Textures/plain.png");      plainTexture.LoadTextureA();
    pisoTexture   = Texture("Textures/piso.tga");       pisoTexture.LoadTextureA();
    AgaveTexture  = Texture("Textures/Agave.tga");      AgaveTexture.LoadTextureA();
    FlechaTexture = Texture("Textures/flechas.tga");    FlechaTexture.LoadTextureA();
    NumerosTexture= Texture("Textures/numerosbase.tga");NumerosTexture.LoadTextureA();
    Numero1Texture= Texture("Textures/numero1.tga");    Numero1Texture.LoadTextureA();
    Numero2Texture= Texture("Textures/numero2.tga");    Numero2Texture.LoadTextureA();
    FuegoTex      = Texture("Textures/fuego.png");      FuegoTex.LoadTextureA();
    // NUEVA textura humo
    HumoTex       = Texture("Textures/Humo.png");       HumoTex.LoadTextureA();

    // Modelos
    Kitt_M = Model();           Kitt_M.LoadModel("Models/kitt_optimizado.obj");
    Llanta_M = Model();         Llanta_M.LoadModel("Models/llanta_optimizada.obj");
    Pista_M = Model();          Pista_M.LoadModel("Models/pista.obj");
    Nave_M = Model();           Nave_M.LoadModel("Models/nave.obj");
    Ala_M = Model();            Ala_M.LoadModel("Models/ala.obj");
    Aeolipile_base_M = Model(); Aeolipile_base_M.LoadModel("Models/Aeolipile_base.obj");
    Aeolipile_M = Model();      Aeolipile_M.LoadModel("Models/Aeolipile.obj");

    // Skybox
    std::vector<std::string> skyboxFaces;
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");
    skybox = Skybox(skyboxFaces);

    // Materiales
    Material_brillante = Material(4.0f, 256);
    Material_opaco     = Material(0.3f, 4);
    Material_metal     = Material(8.0f, 512); // muy brillante para la esfera

    // Luces
    mainLight = DirectionalLight(1.0f,1.0f,1.0f, 0.5f,0.5f, 0.0f,-1.0f,-1.0f);

    unsigned int pointLightCount = 0;
    pointLights[0] = PointLight(1.0f,0.0f,0.0f, 0.0f,1.0f, 0.0f,2.5f,1.5f, 0.3f,0.2f,0.1f);
    pointLightCount++;

    unsigned int spotLightCount = 0;
    spotLights[0] = SpotLight(1.0f,1.0f,1.0f, 0.0f,2.0f, 0.0f,0.0f,0.0f, 0.0f,-1.0f,0.0f, 1.0f,0.0f,0.0f, 5.0f);
    spotLightCount++;
    spotLights[1] = SpotLight(0.0f,0.0f,1.0f, 1.0f,2.0f, 5.0f,10.0f,0.0f, 0.0f,-5.0f,0.0f, 1.0f,0.0f,0.0f, 15.0f);
    spotLightCount++;

    // Uniforms
    GLuint uniformProjection=0, uniformModel=0, uniformView=0, uniformEyePosition=0,
           uniformSpecularIntensity=0, uniformShininess=0, uniformTextureOffset=0;
    GLuint uniformColor=0;
    glm::mat4 projection = glm::perspective(45.0f,
        (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

    // Inicializacion variables originales
    movCoche       = 0.0f;
    movOffset      = 0.1f;
    rotllanta      = 0.0f;
    rotllantaOffset= 10.0f;

    // Inicializar posicion esfera
    posEsfera = posInicialEsfera;

    glm::vec3 lowerLight(0.0f);
    glm::mat4 model(1.0f), modelaux(1.0f);
    glm::vec3 color = glm::vec3(1.0f);
    glm::vec2 toffset = glm::vec2(0.0f);

    // ==========================================
    // LOOP PRINCIPAL
    // ==========================================
    while (!mainWindow.getShouldClose())
    {
        GLfloat now = glfwGetTime();
        // CORRECCION: deltaTime calculado correctamente (error original corregido)
        deltaTime = now - lastTime;
        lastTime  = now;

        // Limitar delta a un maximo para evitar saltos al debuggear
        if (deltaTime > 0.05f) deltaTime = 0.05f;

        angulovaria += 0.5f * deltaTime;

        // Coche
        if (movCoche > -250) {
            movCoche  -= movOffset * deltaTime;
            rotllanta += rotllantaOffset * deltaTime;
        }

        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        fuegoActivo = mainWindow.getfuegoActivo();

        // Numero cambiante
        timerNumeroCambia += deltaTime;
        if (timerNumeroCambia >= intervalNumeroCambia) {
            timerNumeroCambia  = 0.0f;
            toffsetnumerocambiau += 0.25f;
            if (toffsetnumerocambiau >= 1.0f) toffsetnumerocambiau = 0.0f;
        }

        // Aeolipile gira solo si hay fuego
        if (fuegoActivo) rotaAelo += velAelo * deltaTime;

        // Fuego sprite sheet
        if (fuegoActivo) {
            tiempoFuego += deltaTime;
            if (tiempoFuego >= intervaloFuego) {
                tiempoFuego = 0.0f;
                frameFuegoU++;
                if (frameFuegoU >= 4) { frameFuegoU = 0; frameFuegoV++; }
                if (frameFuegoV >= 3)   frameFuegoV = 0;
            }
            toffsetFuegoU = frameFuegoU * 0.25f;
            toffsetFuegoV = frameFuegoV * 0.25f;
        }

        // NUEVAS ACTUALIZACIONES
        ActualizarHumo(deltaTime);
        ActualizarBrazo(deltaTime);
        ActualizarEsfera(deltaTime);

        // ==========================================
        // RENDER
        // ==========================================
        glClearColor(0.0f,0.0f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        skybox.DrawSkybox(camera.calculateViewMatrix(), projection);

        shaderList[0].UseShader();
        uniformModel            = shaderList[0].GetModelLocation();
        uniformProjection       = shaderList[0].GetProjectionLocation();
        uniformView             = shaderList[0].GetViewLocation();
        uniformEyePosition      = shaderList[0].GetEyePositionLocation();
        uniformColor            = shaderList[0].getColorLocation();
        uniformTextureOffset    = shaderList[0].getOffsetLocation();
        uniformSpecularIntensity= shaderList[0].GetSpecularIntensityLocation();
        uniformShininess        = shaderList[0].GetShininessLocation();

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
        glUniform3f(uniformEyePosition,
            camera.getCameraPosition().x,
            camera.getCameraPosition().y,
            camera.getCameraPosition().z);

        lowerLight = camera.getCameraPosition();
        lowerLight.y -= 0.3f;
        spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

        shaderList[0].SetDirectionalLight(&mainLight);
        shaderList[0].SetPointLights(pointLights, pointLightCount);
        shaderList[0].SetSpotLights(spotLights, spotLightCount);

        // Reset de variables de render
        model   = glm::mat4(1.0f);
        modelaux= glm::mat4(1.0f);
        color   = glm::vec3(1.0f);
        toffset = glm::vec2(0.0f);
        glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));

        // ---- PISO ----
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f,-2.0f,0.0f));
        model = glm::scale(model, glm::vec3(30.0f,1.0f,30.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
        pisoTexture.UseTexture();
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[2]->RenderMesh();

        // ---- COCHE ----
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(movCoche-50.0f, 0.5f,-2.0f));
        modelaux = model;
        model = glm::scale(model, glm::vec3(0.5f,0.5f,0.5f));
        model = glm::rotate(model, -90*toRadians, glm::vec3(0.0f,1.0f,0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Kitt_M.RenderModel();

        // Llantas (delantera izq)
        model = modelaux;
        model = glm::translate(model, glm::vec3(7.0f,-0.5f,8.0f));
        model = glm::rotate(model, -90*toRadians, glm::vec3(0.0f,1.0f,0.0f));
        model = glm::rotate(model, rotllanta*toRadians, glm::vec3(1.0f,0.0f,0.0f));
        model = glm::scale(model, glm::vec3(0.4f,0.4f,0.4f));
        color = glm::vec3(0.5f,0.5f,0.5f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Llanta_M.RenderModel();

        // Llanta trasera izquierda
        model = modelaux;
        model = glm::translate(model, glm::vec3(15.5f,-0.5f,8.0f));
        model = glm::rotate(model, -90*toRadians, glm::vec3(0.0f,1.0f,0.0f));
        model = glm::rotate(model, rotllanta*toRadians, glm::vec3(1.0f,0.0f,0.0f));
        model = glm::scale(model, glm::vec3(0.4f,0.4f,0.4f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Llanta_M.RenderModel();

        // Llanta delantera derecha
        model = modelaux;
        model = glm::translate(model, glm::vec3(7.0f,-0.5f,1.5f));
        model = glm::rotate(model, 90*toRadians, glm::vec3(0.0f,1.0f,0.0f));
        model = glm::rotate(model, -rotllanta*toRadians, glm::vec3(1.0f,0.0f,0.0f));
        model = glm::scale(model, glm::vec3(0.4f,0.4f,0.4f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Llanta_M.RenderModel();

        // Llanta trasera derecha
        model = modelaux;
        model = glm::translate(model, glm::vec3(15.5f,-0.5f,1.5f));
        model = glm::rotate(model, 90*toRadians, glm::vec3(0.0f,1.0f,0.0f));
        model = glm::rotate(model, -rotllanta*toRadians, glm::vec3(1.0f,0.0f,0.0f));
        model = glm::scale(model, glm::vec3(0.4f,0.4f,0.4f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Llanta_M.RenderModel();
        color = glm::vec3(1.0f,1.0f,1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        // ---- NAVE (jerarquia correcta) ----
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f,3.0f,1.5f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Nave_M.RenderModel();
        // Ala hereda de la nave
        model = glm::translate(model, glm::vec3(0.0f,0.0f,-1.5f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Ala_M.RenderModel();

        // ---- AEOLIPILE (jerarquia corregida) ----
        // La base define la posicion del sistema completo — escalada mas grande
        glm::mat4 modelAeloBase = glm::mat4(1.0f);
        modelAeloBase = glm::translate(modelAeloBase, glm::vec3(0.0f,-0.5f,1.5f));
        modelAeloBase = glm::scale(modelAeloBase, glm::vec3(1.8f, 1.8f, 1.8f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelAeloBase));
        Aeolipile_base_M.RenderModel();

        // La esfera del Aeolipile HEREDA de la base y rota sobre su propio eje Z
        // (giro tipo "noria" / rueda, como en el gif de referencia)
        glm::mat4 modelAeloEsfera = modelAeloBase; // hereda de la base (incluyendo escala)
        modelAeloEsfera = glm::translate(modelAeloEsfera, glm::vec3(0.0f,4.0f,0.0f));
        modelAeloEsfera = glm::rotate(modelAeloEsfera, rotaAelo*toRadians, glm::vec3(0.0f,0.0f,1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelAeloEsfera));
        Aeolipile_M.RenderModel();

        // ---- DETECTOR (visualizacion) ----
        // Un pequeño cubo/esfera que "brilla" cuando el detector se activa
        if (!brazolActivado && !brazoDisparado)
        {
            model = modelAeloBase; // cerca del Aeolipile
            model = glm::translate(model, glm::vec3(1.2f, 4.5f, 0.0f));
            model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            // Color verde = esperando; rojo = activado
            color = glm::vec3(0.0f, 1.0f, 0.0f);
            glUniform3fv(uniformColor, 1, glm::value_ptr(color));
            plainTexture.UseTexture();
            Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
            meshList[0]->RenderMesh();
        }
        else
        {
            model = modelAeloBase;
            model = glm::translate(model, glm::vec3(1.2f, 4.5f, 0.0f));
            model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            color = glm::vec3(1.0f, 0.2f, 0.0f);
            glUniform3fv(uniformColor, 1, glm::value_ptr(color));
            plainTexture.UseTexture();
            Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
            meshList[0]->RenderMesh();
        }
        color = glm::vec3(1.0f,1.0f,1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        // ---- BRAZO / PALANCA (catapulta) ----
        // El mesh tiene longitud en +Y (y=0 es la BASE/pivot, y=2 es la PUNTA).
        // Queremos:
        //   - Base (pivot) anclada junto al Aeolipile
        //   - Brazo recostado horizontalmente en reposo (punta apunta hacia +X exterior)
        //   - La animacion rota DESDE la base: se levanta la punta hacia arriba (carga) y cae hacia abajo (lanzamiento)
        //
        // Orden de transformaciones (se aplican de abajo hacia arriba en el codigo):
        //   1. Posicionar el pivot en el mundo (junto al Aeolipile, a su lado exterior)
        //   2. Aplicar la animacion de carga/lanzamiento (rota en Z desde el pivot)
        //   3. Orientar el mesh: -90 en Z para que la punta (+Y local) apunte hacia +X (recostado horizontal)
        {
            glm::mat4 modelBrazo = glm::mat4(1.0f);
            // 1. Pivot en el mundo: al lado del Aeolipile escalado, a la altura del suelo
            modelBrazo = glm::translate(modelBrazo, glm::vec3(2.8f, -0.5f, 1.5f));

            // 2. Animacion: carga sube la punta (angulo positivo en Z), lanzamiento la baja
            modelBrazo = glm::rotate(modelBrazo, anguloBrazo * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));

            // 3. Orientacion base del mesh: -90 en Z pone la longitud (+Y) horizontal apuntando a +X
            modelBrazo = glm::rotate(modelBrazo, -90.0f * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
            modelBrazo = glm::scale(modelBrazo, glm::vec3(2.0f, 2.5f, 2.0f));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelBrazo));
            color = glm::vec3(0.6f, 0.35f, 0.1f);
            glUniform3fv(uniformColor, 1, glm::value_ptr(color));
            plainTexture.UseTexture();
            Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
            meshList[8]->RenderMesh();
        }
        color = glm::vec3(1.0f,1.0f,1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        // ---- ESFERA METALICA ----
        // Solo se dibuja si NO esta dentro del canal (bajo el piso)
        if (!esferaEnCanal)
        {
            toffset = glm::vec2(0.0f, 0.0f);
            glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
            model = glm::mat4(1.0f);
            model = glm::translate(model, posEsfera);
            model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            color = glm::vec3(0.8f, 0.8f, 0.9f); // color metalico plateado
            glUniform3fv(uniformColor, 1, glm::value_ptr(color));
            plainTexture.UseTexture();
            Material_metal.UseMaterial(uniformSpecularIntensity, uniformShininess);
            meshList[9]->RenderMesh(); // quad esfera
        }
        color = glm::vec3(1.0f,1.0f,1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        // ---- FUEGO (sprite animado con blending) ----
        if (fuegoActivo)
        {
            toffset = glm::vec2(toffsetFuegoU, toffsetFuegoV);
            glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f,0.5f,1.5f));
            model = glm::rotate(model, 90*toRadians, glm::vec3(1.0f,0.0f,0.0f));
            model = glm::scale(model, glm::vec3(2.0f,2.0f,2.0f));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            color = glm::vec3(1.0f,1.0f,1.0f);
            glUniform3fv(uniformColor, 1, glm::value_ptr(color));
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            FuegoTex.UseTexture();
            Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
            meshList[4]->RenderMesh();
            glDisable(GL_BLEND);
        }

        // ---- HUMO (sprite animado con blending, sale de los tubos del Aeolipile) ----
        // Los tubos del Aeolipile estan a los lados de la esfera giratoria.
        // El humo hereda la transformacion de modelAeloEsfera para salir de los tubos.
        if (humoVisible && humoEscala > 0.01f)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            HumoTex.UseTexture();
            color = glm::vec3(0.7f,0.7f,0.7f);
            glUniform3fv(uniformColor, 1, glm::value_ptr(color));
            Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

            // Tubo 1: extremo derecho del Aeolipile (offset local +X, +Y del tubo)
            toffset = glm::vec2(toffsetHumoU, toffsetHumoV);
            glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
            model = modelAeloEsfera; // hereda rotacion del Aeolipile
            model = glm::translate(model, glm::vec3(0.8f, 0.5f, 0.0f));
            model = glm::rotate(model, 90*toRadians, glm::vec3(1.0f,0.0f,0.0f));
            model = glm::scale(model, glm::vec3(humoEscala*0.6f, humoEscala*0.6f, humoEscala*0.6f));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            meshList[7]->RenderMesh();

            // Tubo 2: extremo izquierdo (opuesto al tubo 1)
            model = modelAeloEsfera;
            model = glm::translate(model, glm::vec3(-0.8f, -0.5f, 0.0f));
            model = glm::rotate(model, 90*toRadians, glm::vec3(1.0f,0.0f,0.0f));
            model = glm::scale(model, glm::vec3(humoEscala*0.6f, humoEscala*0.6f, humoEscala*0.6f));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            meshList[7]->RenderMesh();

            glDisable(GL_BLEND);
        }

        // Reset offset para el resto de elementos
        toffset = glm::vec2(0.0f, 0.0f);
        glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
        color = glm::vec3(1.0f,1.0f,1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        // ---- FLECHA animada ----
        toffsetflechau += 0.001f;
        if (toffsetflechau > 1.0f) toffsetflechau = 0.0f;
        toffset = glm::vec2(toffsetflechau, 0.0f);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.0f,1.0f,-6.0f));
        model = glm::rotate(model, 90*toRadians, glm::vec3(1.0f,0.0f,0.0f));
        model = glm::scale(model, glm::vec3(3.0f,3.0f,3.0f));
        glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(1.0f,0.0f,0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        FlechaTexture.UseTexture();
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[4]->RenderMesh();

        // ---- NUMEROS ----
        toffsetnumerou = 0.0f; toffsetnumerov = 0.0f;
        toffset = glm::vec2(toffsetnumerou, toffsetnumerov);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-6.0f,2.0f,-6.0f));
        model = glm::rotate(model, 90*toRadians, glm::vec3(1.0f,0.0f,0.0f));
        model = glm::scale(model, glm::vec3(3.0f,3.0f,3.0f));
        glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(1.0f,1.0f,1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        NumerosTexture.UseTexture();
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[5]->RenderMesh();

        // Numero 1
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-10.0f,2.0f,-6.0f));
        model = glm::rotate(model, 90*toRadians, glm::vec3(1.0f,0.0f,0.0f));
        model = glm::scale(model, glm::vec3(3.0f,3.0f,3.0f));
        toffset = glm::vec2(0.0f,0.0f);
        glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        NumerosTexture.UseTexture();
        meshList[6]->RenderMesh();

        // Numeros 2-4
        for (int i = 1; i < 4; i++) {
            toffsetnumerou += 0.25f;
            toffset = glm::vec2(toffsetnumerou, 0.0f);
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-10.0f-(i*3.0f),2.0f,-6.0f));
            model = glm::rotate(model, 90*toRadians, glm::vec3(1.0f,0.0f,0.0f));
            model = glm::scale(model, glm::vec3(3.0f,3.0f,3.0f));
            glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            NumerosTexture.UseTexture();
            meshList[6]->RenderMesh();
        }

        // Numeros 5-8
        for (int j = 1; j < 5; j++) {
            toffsetnumerou += 0.25f;
            toffset = glm::vec2(toffsetnumerou, -0.33f);
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-7.0f-(j*3.0f),5.0f,-6.0f));
            model = glm::rotate(model, 90*toRadians, glm::vec3(1.0f,0.0f,0.0f));
            model = glm::scale(model, glm::vec3(3.0f,3.0f,3.0f));
            glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            NumerosTexture.UseTexture();
            meshList[6]->RenderMesh();
        }

        // Numero cambiante
        toffset = glm::vec2(toffsetnumerocambiau, 0.0f);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-10.0f,10.0f,-6.0f));
        model = glm::rotate(model, 90*toRadians, glm::vec3(1.0f,0.0f,0.0f));
        model = glm::scale(model, glm::vec3(3.0f,3.0f,3.0f));
        glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(1.0f,1.0f,1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        NumerosTexture.UseTexture();
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[6]->RenderMesh();

        // Numero cambia entre 1 y 2
        toffset = glm::vec2(0.0f,0.0f);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-13.0f,10.0f,-6.0f));
        model = glm::rotate(model, 90*toRadians, glm::vec3(1.0f,0.0f,0.0f));
        model = glm::scale(model, glm::vec3(3.0f,3.0f,3.0f));
        glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        // Alterna entre Numero1 y Numero2 segun timerNumeroCambia
        if (usaNumero)
            Numero1Texture.UseTexture();
        else
            Numero2Texture.UseTexture();
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[5]->RenderMesh();

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}
