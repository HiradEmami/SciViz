#include "View_visualization.h"
#include <rfftw.h>              //the numerical simulation Rlibrary
#include <GL/glut.h>            //the GLUT graphics library       
#include <iostream>				//for printing the help text
#include "Model_color.h"
#include "Model_fftw.h"
#include "Controller_keyboard.h"
#include <GL/glui.h>



View_visualization::View_visualization()
{
	 DIM = 3;
	 color_dir = 0;            //use direction color-coding or not
	 vec_scale = 1000;			//scaling of hedgehogs
	 draw_smoke = 1;           //draw the smoke or not
	 draw_vecs = 0;            //draw the vector field or not
	 colorbar_width = 50;
	 COLOR_BLACKWHITE = 0;
	 COLOR_GRAYSCALE = 1;
	 COLOR_RAINBOW = 2;
	 COLOR_HEATMAP = 3;
	 COLOR_DIVERGING = 4;
	 COLOR_TWOCOLORS = 5;
	 scalar_col = 5;

}



//------ VISUALIZATION CODE STARTS HERE -----------------------------------------------------------------

void View_visualization::set_colormap(Model_color* color,float vy)
{
	vy *= color->NCOLORS; vy = (int)vy; vy /= color->NCOLORS;
	float R, G, B;
	if (scalar_col == COLOR_BLACKWHITE)
		color->blackwhite(vy, &R, &G, &B);
	else if (scalar_col == COLOR_GRAYSCALE)
		color->grayscale(vy, &R, &G, &B);
	else if (scalar_col == COLOR_RAINBOW)
		color->rainbow(vy, &R, &G, &B);
	else if (scalar_col == COLOR_HEATMAP) {
		color->heatmap(vy, &R, &G, &B);
	}
	else if (scalar_col == COLOR_DIVERGING) {
		color->diverging(vy, &R, &G, &B);
	}
	else if (scalar_col == COLOR_TWOCOLORS) {
		color->interpolate(vy, &R, &G, &B, 0, 0, 1, 1, 1, 0);
	}
	float h, s, v;
	color->rgb2hsv(R, G, B, &h, &s, &v);
	color->hsv2rgb(h, s, v, &R, &G, &B);

	glColor3f(R, G, B);

}

//direction_to_color: Set the current color by mapping a direction vector (x,y), using
//                    the color mapping method 'method'. If method==1, map the vector direction
//                    using a rainbow colormap. If method==0, simply use the white color
void View_visualization::direction_to_color(float x, float y, int method)
{
	float r, g, b, f;
	if (method)
	{
		f = atan2(y, x) / 3.1415927 + 1;
		r = f;
		if (r > 1) r = 2 - r;
		g = f + .66667;
		if (g > 2) g -= 2;
		if (g > 1) g = 2 - g;
		b = f + 2 * .66667;
		if (b > 2) b -= 2;
		if (b > 1) b = 2 - b;
	}
	else
	{
		r = g = b = 1;
	}
	glColor3f(r, g, b);
}

//compute the rgb values given the current segment and the current colormap
void View_visualization::compute_RGB(Model_color* color,float value, float* R, float* G, float* B) {
	switch (scalar_col) {
	case 0:
		color->blackwhite(value, R, G, B);
		break;
	case 1:
		color->grayscale(value, R, G, B);
		break;
	case 2:
		color->rainbow(value, R, G, B);
		break;
	case 3:
		color->heatmap(value, R, G, B);
		break;
	case 4:
		color->diverging(value, R, G, B);
		break;
	case 5:
		color->interpolate(value, R, G, B, 0, 0, 1, 1, 1, 0);
		break;
	}


}


//draw a colorbar with the currently selected colormap
void View_visualization::draw_colorbar(Model_color* color) {

	//the amount of 'strips'
	int segments = 50;
	float R, G, B, value;
	//each 'strip' has the same height and width
	float segment_height = winHeight / segments;

	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i < segments + 1; i++) {
		//the value is in the range [0,1], computed by block number / N
		value = (float)i / (float)segments;
		//compute the RGB values using the value of the current strip and the current colormap
		compute_RGB(&*color,value, &R, &G, &B);
		float h, s, v;
		color->rgb2hsv(R, G, B, &h, &s, &v);
		color->hsv2rgb(h, s, v, &R, &G, &B);
		glColor3f(R, G, B);
		//draw the strips with two vertices
		glVertex2f(winWidth - colorbar_width, i*segment_height);
		glVertex2f(winWidth, i*segment_height);
	}

	glEnd();

}

void View_visualization::draw_numbers(Model_color* color) {
	glColor3f(1, 1, 1);
	glRasterPos2f(winWidth - 20, winHeight - 20);

	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, (float)color->scale_max);
}


