#pragma once
#include<stdio.h>
#include<glew.h>
#include<glfw3.h>

class Window
{
public:
	Window();
	Window(GLint windowWidth, GLint windowHeight);
	int Initialise();
	GLfloat getBufferWidth() { return bufferWidth; }
	GLfloat getBufferHeight() { return bufferHeight; }
	bool getShouldClose() {
		return  glfwWindowShouldClose(mainWindow);
	}
	bool* getsKeys() { return keys; }
	GLfloat getXChange();
	GLfloat getYChange();
	void swapBuffers() { return glfwSwapBuffers(mainWindow); }
	GLfloat getrotay() { return rotay; }
	GLfloat getrotax() { return rotax; }
	GLfloat getrotaz() { return rotaz; }
	GLfloat getarticulacion1() { return articulacion1; }
	GLfloat getarticulacion2() { return articulacion2; }
	GLfloat getarticulacion3() { return articulacion3; }
	GLfloat getarticulacion4() { return articulacion4; }
	GLfloat getarticulacion5() { return articulacion5; }
	GLfloat getarticulacion6() { return articulacion6; }

	// Nuevas articulaciones
	GLfloat getarticCabeza() { return articCabeza; }
	GLfloat getarticOrejaDer() { return articOrejaDer; }
	GLfloat getarticOrejaIzq() { return articOrejaIzq; }
	GLfloat getarticCola() { return articCola; }
	GLfloat getarticPataDelDer1() { return articPataDelDer1; }
	GLfloat getarticPataDelDer2() { return articPataDelDer2; }
	GLfloat getarticPataDelIzq1() { return articPataDelIzq1; }
	GLfloat getarticPataDelIzq2() { return articPataDelIzq2; }
	GLfloat getarticPataTrasDer1() { return articPataTrasDer1; }
	GLfloat getarticPataTrasDer2() { return articPataTrasDer2; }
	GLfloat getarticPataTrasIzq1() { return articPataTrasIzq1; }
	GLfloat getarticPataTrasIzq2() { return articPataTrasIzq2; }

	~Window();
private:
	GLFWwindow* mainWindow;
	GLint width, height;
	GLfloat rotax, rotay, rotaz, articulacion1, articulacion2, articulacion3, articulacion4, articulacion5, articulacion6;

	// Nuevas articulaciones
	GLfloat articCabeza, articOrejaDer, articOrejaIzq, articCola;
	GLfloat articPataDelDer1, articPataDelDer2, articPataDelIzq1, articPataDelIzq2;
	GLfloat articPataTrasDer1, articPataTrasDer2, articPataTrasIzq1, articPataTrasIzq2;

	bool keys[1024];
	GLint bufferWidth, bufferHeight;
	GLfloat lastX;
	GLfloat lastY;
	GLfloat xChange;
	GLfloat yChange;
	bool mouseFirstMoved;
	void createCallbacks();
	static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
	static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);
};