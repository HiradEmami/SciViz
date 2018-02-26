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
	 draw_vecs = 1;            //draw the vector field or not
	 colorbar_width = 50;
	 COLOR_BLACKWHITE = 0;
	 COLOR_GRAYSCALE = 1;
	 COLOR_RAINBOW = 2;
	 COLOR_HEATMAP = 3;
	 COLOR_DIVERGING = 4;
	 COLOR_TWOCOLORS = 5;
	 scalar_col = 0;

}


//------ VISUALIZATION CODE STARTS HERE -----------------------------------------------------------------

void View_visualization::set_colormap(Model_color* color,float vy, int dataset)
{

	// change scale parameters depending on dataset
	if (dataset == 0 ) {
		color->max = color->density_max;
		color->min = color->density_min;
	}
	else if (dataset == 1) {
		color->max = color->vel_max;
		color->min = color->vel_min;
	}

	vy = color->clamp(vy);
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

//direction_to_color: Set the current color by mapping the magnitude of a direction vector (x,y), using
//the selected scalar dataset and colormap                    
void View_visualization::direction_to_color(float x, float y, float scalar, int colormap, Model_color color)
{
	scalar = color.clamp(scalar);
	float r, g, b, f;
	if (color_dir) {
		if (colormap == COLOR_BLACKWHITE)
		{
			// get orientation
			//f = atan2(y, x) / 3.1415927 + 1;

			/*r = f;
			if (r > 1) r = 2 - r;
			g = f + .66667;
			if (g > 2) g -= 2;
			if (g > 1) g = 2 - g;
			b = f + 2 * .66667;
			if (b > 2) b -= 2;
			if (b > 1) b = 2 - b;*/
			color.blackwhite(scalar, &r, &g, &b);
		}
		else if (colormap == COLOR_GRAYSCALE) {
			color.grayscale(scalar, &r, &g, &b);
		}
		else if (colormap == COLOR_RAINBOW) {
			color.rainbow(scalar, &r, &g, &b);
		}
		else if (colormap == COLOR_HEATMAP) {
			color.heatmap(scalar, &r, &g, &b);
		}
		else if (colormap == COLOR_DIVERGING) {
			color.diverging(scalar, &r, &g, &b);
		}
		else if (colormap == COLOR_TWOCOLORS) {
			color.interpolate(scalar, &r, &g, &b, 0, 0, 1, 1, 1, 0);
		}
	}
	else {
		r = g = b = 1;
	}
	glColor3f(r, g, b);
}

//compute the rgb values given the current segment and the current colormap
void View_visualization::compute_RGB(Model_color* color,float value, float* R, float* G, float* B) {
	value = color->clamp(value);
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
	//int segments = 3;
	float R, G, B, value;
	//each 'strip' has the same height and width
	//float segment_height = winHeight / segments;

	glBegin(GL_QUAD_STRIP);

		// draw colorbar by using three sets of two vertices
		value = 0;
		compute_RGB(&*color, value, &R, &G, &B);
		float h, s, v;
		color->rgb2hsv(R, G, B, &h, &s, &v);
		color->hsv2rgb(h, s, v, &R, &G, &B);
		glColor3f(R, G, B);
		glVertex2f(colorbar_width, 0);
		glVertex2f(0, 0);

		value = 0.5;
		compute_RGB(&*color, value, &R, &G, &B);
		color->rgb2hsv(R, G, B, &h, &s, &v);
		color->hsv2rgb(h, s, v, &R, &G, &B);
		glColor3f(R, G, B);
		glVertex2f(colorbar_width, winHeight / 2);
		glVertex2f(0, winHeight / 2);

		value = 1.0;
		compute_RGB(&*color, value, &R, &G, &B);
		color->rgb2hsv(R, G, B, &h, &s, &v);
		color->hsv2rgb(h, s, v, &R, &G, &B);
		glColor3f(R, G, B);
		glVertex2f(colorbar_width, winHeight);
		glVertex2f(0, winHeight);
	/*for (int i = 0; i < segments + 1; i++) {
		//the value is in the range [0,1], computed by block number / N
		value = (float)i / (float)segments;
		//compute the RGB values using the value of the current strip and the current colormap
		compute_RGB(&*color,value, &R, &G, &B);
		float h, s, v;
		color->rgb2hsv(R, G, B, &h, &s, &v);
		color->hsv2rgb(h, s, v, &R, &G, &B);
		glColor3f(R, G, B);
		//draw the strips with two vertices
		glVertex2f(colorbar_width, i*segment_height);
		glVertex2f(0, i*segment_height); 
	}*/
	glEnd();

	//draw scale of color bar
	/*draw_number(color, (char)(color->min), 0);
	draw_number(color, (char)(color->max), winHeight);
	draw_number(color, (char)((color->min + color->min) / 2), winHeight / 2);

	draw_number(color, '0', 0);
	draw_number(color, '1', winHeight - 10);
	draw_number(color, '0.5', winHeight / 2);*/

}

void View_visualization::draw_number(Model_color* color, char value, float position) {
	glColor3f(1, 1, 1);
	glRasterPos2f(colorbar_width / 2, position);
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, value); 
	
}


