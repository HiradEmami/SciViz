#pragma once
#include <rfftw.h>              //the numerical simulation Rlibrary
#include <GL/glut.h>            //the GLUT graphics library       
#include <iostream>				//for printing the help text
#include "Model_color.h"
#include "Model_fftw.h"
//#include "Controller_keyboard.h"
#include <GL/glui.h>

class View_visualization
{
public:
	View_visualization();

	void draw_number(Model_color* color, std::string value, float position);
	void draw_colorbar(Model_color* color);
	void compute_RGB(Model_color* color,float value, float* R, float* G, float* B);
	void direction_to_color(float scalar, int colormap, Model_color color);
	void set_colormap(Model_color* color, float vy, int dataset);
	void set_Glyph_type();
	void draw2Dglyph();
	void get_reference_coordinates(int p, int p1, int p2, int p3, int p4, int* r, int* s);
	void drawCircle(GLfloat cx, GLfloat cy, GLfloat radius);
	void display_Steamline(Model_fftw* model_fft);
	void visualize(int DIM, Model_fftw* model_fft, Model_color* color,int* DENSITY, int* VELOCITY, int* FORCE, int* dataset,
		int* SCALAR_DENSITY, int* SCALAR_VELOCITY, int* SCALAR_FORCE, int* dataset_scalar,
		int* VECTOR_VELOCITY, int* VECTOR_FORCE, int* dataset_vector);
	//parameters
	int DIM;				//size of simulation grid
	int   winWidth, winHeight;  //size of the graphics window, in pixels
	int   color_dir;            //use direction color-coding or not
	float vec_scale;			//scaling of hedgehogs
	int   draw_smoke;           //draw the smoke or not
	int   draw_vecs;            //draw the vector field or not
	//different types of color mapping: black-and-white, grayscale, rainbow, banded
	int COLOR_BLACKWHITE;   
	int COLOR_GRAYSCALE;
	int COLOR_RAINBOW;
	int COLOR_HEATMAP;
	int COLOR_DIVERGING;
	int COLOR_TWOCOLORS;
	float colorbar_width;
	int colorbar_height;
	int scalar_col;   
	int use_clamp;
	//glyph parameters
	int glyph_type;
	int glyph_samplingrateX;
	int glyph_samplingrateY;
	int vector_type;
	int STANDARD;
	int GRADIENT;
	//Steamline parameters
	GLfloat MOUSEx, MOUSEy;
	int draw_steamline;
};

