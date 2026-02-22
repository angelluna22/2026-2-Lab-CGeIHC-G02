#include <stdio.h>
#include <string.h>
#include <glew.h>
#include <glfw3.h>
#include <math.h>  // ADICIÓN: para funciones matemáticas si son necesarias

//Dimensiones de la ventana
const int WIDTH = 800, HEIGHT = 800;
GLuint VAO, VBO, shader;

// ADICIÓN: Variables para el cambio de color de fondo
float colorRojo = 1.0f;
float colorVerde = 0.0f;
float colorAzul = 0.0f;
int contadorColor = 0;
double tiempoAnterior = 0.0;
const double INTERVALO_CAMBIO = 1.0; // 1 segundo entre cambios

//"VAO sirve para saber los vértices para dibujar puntos, líneas"
//VBO Memoria para guardar un 

//LENGUAJE DE SHADER (SOMBRAS) GLSL
//Vertex Shader
//recibir color, salida Vcolor
static const char* vShader = "						\n\
#version 330										\n\
layout (location =0) in vec3 pos;					\n\
void main()											\n\
{													\n\
gl_Position=vec4(pos.x,pos.y,pos.z,1.0f); 			\n\
}";

//Fragment Shader MODIFICADO: ahora las figuras serán negras
static const char* fShader = "						\n\
#version 330										\n\
out vec4 color;										\n\
void main()											\n\
{													\n\
	color = vec4(1.0f, 1.0f, 1.0f, 1.0f);	 		\n\
}";

// ADICIÓN: Nueva función para crear rombo y trapecio con triángulos
void CrearRomboYTrapecio()
{
	// Vértices para ROMBO (forma de diamante) - AHORA CON TRIÁNGULOS (6 vértices = 2 triángulos)
	// MODIFICACIÓN: Ajusté las coordenadas para mejor visualización con triángulos
	GLfloat vertices[] = {
		// ROMBO (parte superior) - 2 triángulos
		// Triángulo superior
		 0.0f,  0.6f, 0.0f,  // Vértice superior
		 0.4f,  0.2f, 0.0f,  // Vértice derecho superior
		-0.4f,  0.2f, 0.0f,  // Vértice izquierdo superior

		// Triángulo inferior
		 0.0f, -0.2f, 0.0f,  // Vértice inferior
		 0.4f,  0.2f, 0.0f,  // Vértice derecho superior
		-0.4f,  0.2f, 0.0f,  // Vértice izquierdo superior

		// TRAPECIO ISÓSCELES (parte inferior) - AHORA CON TRIÁNGULOS (6 vértices = 2 triángulos)
		// MODIFICADO para mejor forma con triángulos
		// Triángulo izquierdo del trapecio
		-0.5f, -0.3f, 0.0f,  // Base inferior izquierda
		-0.3f, -0.7f, 0.0f,  // Base superior izquierda
		 0.3f, -0.7f, 0.0f,  // Base superior derecha

		 // Triángulo derecho del trapecio
		 -0.5f, -0.3f, 0.0f,  // Base inferior izquierda
		  0.3f, -0.7f, 0.0f,  // Base superior derecha
		  0.5f, -0.3f, 0.0f   // Base inferior derecha
	};

	//Estas variables se guardan en un VAO
	glGenVertexArrays(1, &VAO); //generar 1 VAO
	glBindVertexArray(VAO);//asignar VAO

	//Crear un VBO y llamarlo a buffer

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //pasarle los datos al VBO asignando tamano, los datos y en este caso es estático pues no se modificarán los valores

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0);//Stride en caso de haber datos de color por ejemplo, es saltar cierta cantidad de datos
	glEnableVertexAttribArray(0);
	//agregar valores a vèrtices y luego declarar un nuevo vertexAttribPointer
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void AddShader(GLuint theProgram, const char* shaderCode, GLenum shaderType) //Función para agregar los shaders a la tarjeta gráfica

//the Program recibe los datos de theShader

{
	GLuint theShader = glCreateShader(shaderType);//theShader es un shader que se crea de acuerdo al tipo de shader: vertex o fragment
	const GLchar* theCode[1];
	theCode[0] = shaderCode;//shaderCode es el texto que se le pasa a theCode
	GLint codeLength[1];
	codeLength[0] = strlen(shaderCode);//longitud del texto
	glShaderSource(theShader, 1, theCode, codeLength);//Se le asigna al shader el código
	glCompileShader(theShader);//Se comila el shader
	GLint result = 0;
	GLchar eLog[1024] = { 0 };
	//verificaciones y prevención de errores
	glGetShaderiv(theShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
		printf("EL error al compilar el shader %d es: %s \n", shaderType, eLog);
		return;
	}
	glAttachShader(theProgram, theShader);//Si no hubo problemas se asigna el shader a theProgram el cual asigna el código a la tarjeta gráfica
}

void CompileShaders() {
	shader = glCreateProgram(); //se crea un programa
	if (!shader)
	{
		printf("Error creando el shader");
		return;
	}
	AddShader(shader, vShader, GL_VERTEX_SHADER);//Agregar vertex shader
	AddShader(shader, fShader, GL_FRAGMENT_SHADER);//Agregar fragment shader
	//Para terminar de linkear el programa y ver que no tengamos errores
	GLint result = 0;
	GLchar eLog[1024] = { 0 };
	glLinkProgram(shader);//se linkean los shaders a la tarjeta gráfica
	//verificaciones y prevención de errores
	glGetProgramiv(shader, GL_LINK_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
		printf("EL error al linkear es: %s \n", eLog);
		return;
	}
	glValidateProgram(shader);
	glGetProgramiv(shader, GL_VALIDATE_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
		printf("EL error al validar es: %s \n", eLog);
		return;
	}



}
int main()
{
	//Inicialización de GLFW
	if (!glfwInit())
	{
		printf("Falló inicializar GLFW");
		glfwTerminate();
		return 1;
	}

	//****  LAS SIGUIENTES 4 LÍNEAS SE COMENTAN EN DADO CASO DE QUE AL USUARIO NO LE FUNCIONE LA VENTANA Y PUEDA CONOCER LA VERSIÓN DE OPENGL QUE TIENE ****/

	//Asignando variables de GLFW y propiedades de ventana
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//para solo usar el core profile de OpenGL y no tener retrocompatibilidad
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//CREAR VENTANA
	GLFWwindow* mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "Rombo y Trapecio con fondo ciclico (Triangulos)", NULL, NULL); // ADICIÓN: cambié el título

	if (!mainWindow)
	{
		printf("Fallo en crearse la ventana con GLFW");
		glfwTerminate();
		return 1;
	}
	//Obtener tamaño de Buffer
	int BufferWidth, BufferHeight;
	glfwGetFramebufferSize(mainWindow, &BufferWidth, &BufferHeight);

	//asignar el contexto
	glfwMakeContextCurrent(mainWindow);

	//permitir nuevas extensiones
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		printf("Falló inicialización de GLEW");
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 1;
	}

	// Asignar valores de la ventana y coordenadas
	//Asignar Viewport
	glViewport(0, 0, BufferWidth, BufferHeight);

	//Llamada a las funciones creadas antes del main
	CrearRomboYTrapecio(); // ADICIÓN: cambié la función a la nueva
	CompileShaders();

	// ADICIÓN: Inicializar tiempo
	tiempoAnterior = glfwGetTime();

	//Loop mientras no se cierra la ventana
	while (!glfwWindowShouldClose(mainWindow))
	{
		//Recibir eventos del usuario
		glfwPollEvents();

		// ADICIÓN: Cambiar color de fondo cíclicamente
		double tiempoActual = glfwGetTime();
		if (tiempoActual - tiempoAnterior >= INTERVALO_CAMBIO)
		{
			contadorColor = (contadorColor + 1) % 3;

			if (contadorColor == 0) // Rojo
			{
				colorRojo = 1.0f;
				colorVerde = 0.0f;
				colorAzul = 0.0f;
			}
			else if (contadorColor == 1) // Verde
			{
				colorRojo = 0.0f;
				colorVerde = 1.0f;
				colorAzul = 0.0f;
			}
			else if (contadorColor == 2) // Azul
			{
				colorRojo = 0.0f;
				colorVerde = 0.0f;
				colorAzul = 1.0f;
			}

			tiempoAnterior = tiempoActual;
		}

		//Limpiar la ventana (ADICIÓN: usando las variables de color)
		glClearColor(colorRojo, colorVerde, colorAzul, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shader);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 12); // MODIFICACIÓN: cambié de GL_LINES a GL_TRIANGLES y de 16 a 12 vértices (6 para rombo + 6 para trapecio)
		glBindVertexArray(0);

		glUseProgram(0);

		glfwSwapBuffers(mainWindow);

		//NO ESCRIBIR NINGUNA LÍNEA DESPUÉS DE glfwSwapBuffers(mainWindow); 
	}


	return 0;
}