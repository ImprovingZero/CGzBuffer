﻿// CGzBuffer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include"common.h"
#include"objReader.h"
#include"Model.h"
#include"camera.h"
#include"PolyList.h"
#include"..//inc/GL/glut.h"
#include"ZBuffer.h"

ZBuffer* img;
Model model;

void myInit(void) {
	glColor3f(0.0f, 0.0f, 0.0f);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0, U_PIX_NUM, 0, V_PIX_NUM);
}

void displayFnc() {
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_POINTS);
	for (int i = 0; i < U_PIX_NUM; i++) {
		for (int j = 0; j < V_PIX_NUM; j++) {
			int id = img->_output[j][i];
			vec3 c;
			if (id != -1) c = model._face[id]._color;
			else c = vec3(0.7f, 0.7f, 0.3f);
			glColor3f(c[0], c[1], c[2]);
				//int id = z.buffer[j][i];
				//glColor3f(z.F[id]._color[0], z.F[id]._color[1], z.F[id]._color[2]);
			glVertex2f(i, j);
		}
	}
	glEnd();
	glFlush();
}

int main(int argc, char* argv[])
{
    std::cout << "Hello World!\n";
	objReader fileReader("0000_00#1.obj");
	
	fileReader.read(model._face, model._pos, model._nml, model._tex);
	
	camera cam(vec3(0.f, -50.f, 0.f), vec3(0.f, 0.f, -1.f), vec3(0.f, 1.f, 0.f), 90, 1.5,20.f);
	cam._u = vec3(1.f, 0.f, 0.f);
	cam._v = vec3(0.f, 1.f, 0.f);
	cam._w = vec3(0.f, 0.f, -1.f);
	PolyList pll(&model, &cam);

	img = new ZBuffer(&pll);
	img->generate();
	std::cout << cam._u << std::endl;
	std::cout << cam._v << std::endl;
	std::cout << cam._w << std::endl;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(U_PIX_NUM, V_PIX_NUM);
	glutInitWindowPosition(200, 200);
	glutCreateWindow("zbuffer");
	myInit();
	glutDisplayFunc(displayFnc);
	glutMainLoop();
	

}