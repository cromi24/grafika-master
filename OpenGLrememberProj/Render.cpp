#include "Render.h"

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"
#include "math.h"
#include "Camera.h"
#include "Light.h"
#include "Primitives.h"



bool textureMode = true;
bool lightMode = true;
GLuint texId[2];

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(0, 0, 5);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor4d(0.9, 0.8, 0, 1);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor4f(0, 0, 0, 1);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}




//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;


	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;

	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

		
//генерируем ИД для текстуры
	glGenTextures(2, texId);
//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна

	//отчистка памяти
	free(texCharArray);
	free(texarray);


	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	//////////////////текстура 2
	OpenGL::LoadBMP("texture2.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);
	glBindTexture(GL_TEXTURE_2D, texId[1]);//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
}


void normal(double x1[], double y1[], double z1[], double vecn[])/////////нормали*****
{
	double qx, qy, qz, px, py, pz;

	qx = x1[0] - y1[0];
	qy = x1[1] - y1[1];
	qz = x1[2] - y1[2];
	px = z1[0] - y1[0];
	py = z1[1] - y1[1];
	pz = z1[2] - y1[2];
	vecn[0] = py*qz - pz*qy;
	vecn[1] = pz*qx - px*qz;
	vecn[2] = px*qy - py*qx;

	double length = sqrt(pow(vecn[0], 2) + pow(vecn[1], 2) + pow(vecn[2], 2));

	vecn[0] /= length;
	vecn[1] /= length;
	vecn[2] /= length;
}




void Render(OpenGL *ogl)
{       	


	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);



	//glEnable(GL_ALPHA_TEST);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	

	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);\
	//размер блика
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

    //чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  

	///////////////////

	double a[] = { -4, 4, 0 };
	double a1[] = { -4, 4, 2 };
	double b[] = { 3, 2, 0 };
	double b1[] = { 3, 2, 2 };
	double c[] = { 13, 3, 0 };
	double c1[] = { 13, 3, 2 };
	double d[] = { 9, -4, 0 };
	double d1[] = { 9, -4, 2 };
	double e[] = { -2, -4, 0 };
	double e1[] = { -2, -4, 2 };
	double f[] = { -5, -2, 0 };
	double f1[] = { -5, -2, 2 };
	double g[] = { -3, 2, 0 };
	double g1[] = { -3, 2, 2 };
	double n[] = { 7, 5, 0 };
	double n1[] = { 7, 5, 2 };
	double m[] = { 8, 0, 0 };
	double m1[] = { 8, 0, 2 };
	
	double a3[] = { -2, 0, 5 };
	double b3[] = { 2, 0, 5 };
	double c3[] = { 2, 0, 5 };
	double d3[] = { 0, -2, 5 };
	double o3[] = { 0, 0, 1 };//r= 2
	double o4[] = { 0, 0, -3 };//r= 2
	double o[] = { 8.346153846153847, -0.9615384615384616, 0 };//r= 6.111634070027194
	double o2[] = { 12.06522, -1.10870, 0 };//r= 4.21
	double vecOB[] = { 3 - o[0], 2 - o[1], 0 };
	double vecOC[] = { 13 - o[0], 3 - o[1], 0 };
	double angle1 = acos((vecOB[0] * vecOC[0] + vecOB[1] * vecOC[1]) / pow(6.111634070027194, 2)) / 3.14 * 180;
	double Cvec[] = { 6.111634070027194, 0, 2 };
	double angle2 = acos((Cvec[0] * vecOC[0] + Cvec[1] * vecOC[1]) / pow(6.111634070027194, 2)) / 3.14 * 180;

	double vec2OC[] = { 13 - o2[0], 3 - o2[1], 0 };
	double vec2OD[] = { 9 - o2[0], -4 - o2[1], 0 };
	double Cvec2[] = { 4.21, 0, 2 };
	double angle3 = acos((vec2OD[0] * vec2OC[0] + vec2OD[1] * vec2OC[1]) / pow(4.21, 2)) / 3.14 * 180;
	double angle4 = acos((Cvec2[0] * vec2OC[0] + Cvec2[1] * vec2OC[1]) / pow(4.21, 2)) / 3.14 * 180;

	//нижнее основание
	glBindTexture(GL_TEXTURE_2D, texId[1]);
	glBegin(GL_TRIANGLES);
	glNormal3f(0, 0, -1);
	glColor4f(0,0,1,1);

	glTexCoord2d(0.6640625, 0);
	glVertex3dv(e);
	glTexCoord2d(0.5859375, 0.14453125); 
	glVertex3dv(b);
	glTexCoord2d(0.72265625, 0.0390625);
	glVertex3dv(f);

	glTexCoord2d(0.72265625, 0.0390625);
	glVertex3dv(f);
	glTexCoord2d(0.5859375, 0.14453125);
	glVertex3dv(b);
	glTexCoord2d(0.68359375, 0.140625);
	glVertex3dv(g);

	glTexCoord2d(0.68359375, 0.140625);
	glVertex3dv(g);
	glTexCoord2d(0.703125, 0.1796875);
	glVertex3dv(a);
	glTexCoord2d(0.5859375, 0.14453125);
	glVertex3dv(b);
	
	glEnd();
	//низ выпуклости

	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2d(0.498046875, 0.162109375);
	glVertex3d(8, 4, 0);

	for (double i = angle2; i < angle1 + angle2; i++)
	{
		double x = (0.48828125 + (50 * cos(i*3.1415926 / 180)) / 512.0);
		double y = (0.1015625 + (50 * sin(i*3.1415926 / 180)) / 512.0);
		glTexCoord2d(x, y);
		glVertex3d(o[0] + cos(i * 3.141592653589793 / 180) * 6.111634070027194, o[1] + sin(i * 3.141592653589793 / 180) * 6.111634070027194, 0);
		glTexCoord2d(0.498046875, 0.162109375);
		glVertex3d(8, 4, 0);
	}
	glEnd();

	//низ впуулости
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2d(0.498046875, 0.162109375);
	glVertex3d(8, 4, 0);
	for (double i = angle4-3; i < (angle3 + angle4) - 70; i++){
		double x = (0.37109375 + (50 * cos(i*3.1415926 / 180)) / 512.0);
		double y = (0.0625 + (50 * sin(i*3.1415926 / 180)) / 512.0);
		glTexCoord2d(x, y);
		glVertex3d(o2[0] + cos(i * 3.141592653589793 / 180) * 4.21, o2[1] + sin(i * 3.141592653589793 / 180) * 4.21, 0);
		glTexCoord2d(0.498046875, 0.162109375);
		glVertex3d(8, 4, 0);
	}
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2d(0.5859375, 0.14453125);
	glVertex3dv(b);
	for (double i = (angle3 + angle4) - 70; i < angle3 + angle4+3; i++){
		double x = (0.37109375 + (50 * cos(i*3.1415926 / 180)) / 512.0);
		double y = (0.0625 + (50 * sin(i*3.1415926 / 180)) / 512.0);
		glTexCoord2d(x, y);
		glVertex3d(o2[0] +cos(i * 3.141592653589793 / 180) * 4.21, o2[1] + sin(i * 3.141592653589793 / 180) * 4.21, 0);
		glTexCoord2d(0.5859375, 0.14453125);
		glVertex3dv(b);
	}
	glEnd();

	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2d(0.44921875, 0);
	glVertex3dv(d);
	glTexCoord2d(0.5859375, 0.14453125);
	glVertex3dv(b);
	glTexCoord2d(0.6640625, 0);
	glVertex3dv(e);
	glEnd();

	//бока
	glBindTexture(GL_TEXTURE_2D, texId[1]);
	glBegin(GL_QUADS);
	glColor4f(0, 0, 0.8f, 1);
	double vecn[] = { 0, 0, 0 };
	normal(e, d, d1, vecn);
	glNormal3dv(vecn);

	glTexCoord2d(0.68359375, 0.8056875);
	glVertex3dv(e);
	glTexCoord2d(1.0, 0.8056875);
	glVertex3dv(d);
	glTexCoord2d(1.0, 1.0);
	glVertex3dv(d1);
	glTexCoord2d(0.68359375, 1.0);
	glVertex3dv(e1);
	glEnd();

	glBegin(GL_QUADS);

	normal(b, a, a1, vecn);
	glNormal3dv(vecn);

	glTexCoord2d(0.68359375, 0.8056875);
	glVertex3dv(b);
	glTexCoord2d(1, 0.8056875);
	glVertex3dv(a);
	glTexCoord2d(1, 1);
	glVertex3dv(a1);
	glTexCoord2d(0.68359375, 1);
	glVertex3dv(b1);
	glEnd();

	glBegin(GL_QUADS);

	normal(a, g, g1, vecn);
	glNormal3dv(vecn);

	glTexCoord2d(0, 0.8056875);
	glVertex3dv(a);
	glTexCoord2d(0.1953125, 0.8056875);
	glVertex3dv(g);
	glTexCoord2d(0.1953125, 1);
	glVertex3dv(g1);
	glTexCoord2d(0, 1);
	glVertex3dv(a1);
	glEnd();


	glBegin(GL_QUADS);

	normal(g, f, f1, vecn);
	glNormal3dv(vecn);

	glTexCoord2d(0.68359375, 0.8056875);
	glVertex3dv(g);
	glTexCoord2d(1, 0.8056875);
	glVertex3dv(f);
	glTexCoord2d(1, 1);
	glVertex3dv(f1);
	glTexCoord2d(0.68359375, 1);
	glVertex3dv(g1);
	glEnd();

	glBegin(GL_QUADS);

	normal(f, e, e1, vecn);
	glNormal3dv(vecn);

	glTexCoord2d(0.1953125, 0.8056875);
	glVertex3dv(f);
	glTexCoord2d(0.68359375, 0.8056875);
	glVertex3dv(e);
	glTexCoord2d(0.68359375,1);
	glVertex3dv(e1);
	glTexCoord2d(0.1953125, 1);
	glVertex3dv(f1);
	glEnd();


	////ВЫпуклость
	glBegin(GL_TRIANGLE_STRIP);
	glColor4f(0, 0, 0.8f, 1);
	normal(b, n1, c, vecn);
	glNormal3dv(vecn);


	for (double i = angle2 - (10 * 3.141592653589793 / 180); i < angle1 + angle2; i++)
	{
		glTexCoord2d((0.1953125 + ((i -angle2- (10 * 3.141592653589793 / 180)) / 512.0)), 0.8056875);
			 glVertex3d(o[0] + cos(i * 3.141592653589793 / 180) * 6.111634070027194, o[1] + sin(i * 3.141592653589793 / 180) * 6.111634070027194, 0); 
			 glTexCoord2d(0.1953125 + ((i-angle2 - (10 * 3.141592653589793 / 180)) / 512.0), 1);
			 glVertex3d(o[0] + cos(i * 3.141592653589793 / 180) * 6.111634070027194, o[1] + sin(i * 3.141592653589793 / 180) * 6.111634070027194, 2);
	}
	glEnd();
	/////ВПуклость
	glBegin(GL_TRIANGLE_STRIP);
	glColor4f(0, 0, 0.8f, 1);
	normal(c, m1, d, vecn);
	glNormal3dv(vecn);

	for (double i = angle4 ; i < angle3 + angle4; i++){
		glTexCoord2d((0.1953125 + ((i - angle4) / 512.0)), 0.8056875);
		glVertex3d(o2[0] + cos(i * 3.141592653589793 / 180) * 4.21, o2[1] + sin(i * 3.141592653589793 / 180) * 4.21, 0);
		glTexCoord2d((0.1953125 + ((i -angle4) / 512.0)), 1);
		glVertex3d(o2[0] + cos(i * 3.141592653589793 / 180) * 4.21, o2[1] + sin(i * 3.141592653589793 / 180) * 4.21, 2);
	}
	glEnd();
	//========================текстурированный круг
	glBindTexture(GL_TEXTURE_2D, texId[0]);
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0, 0, 1);
	glColor4f(1, 1, 0, 1);


	for (double i = 0; i <= 360; i++)
	{
		float x = 0.5f + 0.5f * cos(i * 3.1415 / 180);
		float y = 0.5f + 0.5f * sin(i * 3.1415 / 180);


		glTexCoord2d(x, y);
		glVertex3d(2 * cos(i * 3.1415 / 180) + o3[0], 2 * sin(i * 3.1415 / 180) + o3[1], 1);
	}

	glEnd();
	//==========

	//верхнее основание
	glBindTexture(GL_TEXTURE_2D, texId[1]);
	glBegin(GL_TRIANGLES);
	glNormal3f(0,0,1);
	glColor4f(0.0f, 0.0f, 1, 0.4f);

	glTexCoord2d(0.05859375, 0);
	glVertex3dv(e1);
	glTexCoord2d(0.140625, 0.14453125);
	glVertex3dv(b1);
	glTexCoord2d(0, 0.0390625);
	glVertex3dv(f1);

	glTexCoord2d(0, 0.0390625);
	glVertex3dv(f1);
	glTexCoord2d(0.140625, 0.14453125);
	glVertex3dv(b1);
	glTexCoord2d(0.0390625, 0.140625);
	glVertex3dv(g1);

	glTexCoord2d(0.0390625, 0.140625);
	glVertex3dv(g1);
	glTexCoord2d(0.01953125, 0.1796875);
	glVertex3dv(a1);
	glTexCoord2d(0.140625, 0.14453125);
	glVertex3dv(b1);

	glEnd();
	//верх выпуклости
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2d(0.240234375, 0.162109375);
	glVertex3d(8, 4, 2);
	for (double i = angle2 - (10*3.141592653589793 / 180); i < angle1 + angle2 + 3; i++)
	{
		double x = (0.234375 + (50*cos(i*3.1415926 / 180))/512.0);
		double y = (0.1015625 + (50*sin(i*3.1415926 / 180)) / 512.0);
		glTexCoord2d(x,y);
		glVertex3d(o[0] + cos(i * 3.141592653589793 / 180) * 6.111634070027194, o[1] + sin(i * 3.141592653589793 / 180) * 6.111634070027194, 2);
		glTexCoord2d(0.240234375, 0.162109375);
		glVertex3d(8, 4, 2);
	}
	glEnd();
	//верх впуклости

	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2d(0.240234375, 0.162109375);
	glVertex3d(8, 4, 2);
	for (double i = angle4-3; i < (angle3 + angle4) - 70; i++){
		double x = (0.33203125 + (50 * cos(i*3.1415926 / 180)) / 512.0);
		double y = (0.04296875 + (50 * sin(i*3.1415926 / 180)) / 512.0);
		glTexCoord2d(x,y);
		glVertex3d(o2[0] + cos(i * 3.141592653589793 / 180) * 4.21, o2[1] + sin(i * 3.141592653589793 / 180) * 4.21, 2);
		glTexCoord2d(0.240234375, 0.162109375);
		glVertex3d(8, 4, 2);
	}
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2d(0.140625, 0.14453125);
	glVertex3dv(b1);
	for (double i = (angle3 + angle4)-70 ; i < angle3 + angle4+3; i++){
		double x = (0.33203125 + (50 * cos(i*3.1415926 / 180)) / 512.0);
		double y = (0.0625 + (50 * sin(i*3.1415926 / 180)) / 512.0);
		glTexCoord2d(x, y);
		glVertex3d(o2[0] + cos(i * 3.141592653589793 / 180) * 4.21, o2[1] + sin(i * 3.141592653589793 / 180) * 4.21, 2);
		glTexCoord2d(0.140625, 0.14453125);
		glVertex3dv(b1);
	}
	glEnd();
	 
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2d(0.2421875,0);
	glVertex3dv(d1);
	glTexCoord2d(0.140625, 0.14453125);
	glVertex3dv(b1);
	glTexCoord2d(0.05859375,0);
	glVertex3dv(e1);
	glEnd();

	
	

/*	//Начало рисования квадратика станкина
	double A[2] = { -4, -4 };
	double B[2] = { 4, -4 };
	double C[2] = { 4, 4 };
	double D[2] = { -4, 4 };

	
	
	glBegin(GL_QUADS);

	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex2dv(A);
	glTexCoord2d(1, 0);
	glVertex2dv(B);
	glTexCoord2d(1, 1);
	glVertex2dv(C);
	glTexCoord2d(0, 1);
	glVertex2dv(D);

	glEnd();
	//конец рисования квадратика станкина
    */
	
	//текст сообщения вверху слева, если надоест - закоментировать, или заменить =)
	char S[250];  //максимальная длина сообщения
	sprintf_s(S, "(T)Текстуры - %d\n(L)Свет - %d\n\nУправление светом:\n"
		"G - перемещение в горизонтальной плоскости,\nG+ЛКМ+перемещение по вертикальной линии\n"
		"R - установить камеру и свет в начальное положение\n"
		"F - переместить свет в точку камеры", textureMode, lightMode);
	ogl->message = std::string(S);




}   //конец тела функции

