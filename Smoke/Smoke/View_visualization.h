#pragma once
#include <rfftw.h>              //the numerical simulation Rlibrary
#include <GL/glut.h>            //the GLUT graphics library       
#include <iostream>				//for printing the help text
#include "Model_color.h"
#include "Model_fftw.h"
#include "Controller_keyboard.h"
#include <GL/glui.h>

class View_visualization
{
public:
	View_visualization();
	void reshape(int w, int h);
	void display(Model_color* color, Model_fftw* model_fft, int* argdataset, int* argvolacity, int* argDensity);
	void visualize(Model_color* color, Model_fftw* model_fft, int* argdataset, int* argvolacity, int* argDensity);
	void draw_numbers(Model_color* color);
	void draw_colorbar(Model_color* color);
	void compute_RGB(Model_color* color,float value, float* R, float* G, float* B);
	void direction_to_color(float x, float y, int method);
	void set_colormap(Model_color* color,float vy);
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
	int scalar_col;   //set initial colormap to HeatMap

};

