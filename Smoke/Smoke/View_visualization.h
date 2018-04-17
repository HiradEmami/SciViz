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

	void draw_number(Model_color* color, std::string value, float position, float height);
	void draw_colorbar(Model_color* color);
	void compute_RGB(Model_color* color,float value, float* R, float* G, float* B);
	void direction_to_color(float scalar, int colormap, Model_color color);
	void set_colormap(Model_color* color, float vy, int dataset);
	void set_Glyph_type();
	void drawLine(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
	void get_reference_coordinates(double px, double py, double v1x, double v1y, double v2x, double v2y,
		double v4x, double v4y, double* r, double* s);
	void draw_cones(float x, float y, fftw_real  wn, fftw_real hn, int i, int j, float magnitude);
	void draw_arrows(float x, float y, fftw_real  wn, fftw_real hn, int i, int j, float magnitude);
	void rotate(float x, float y, float* newx, float* newy, float pivotx, float pivoty, float angle);
	void drawCircle(GLfloat cx, GLfloat cy, GLfloat radius);
	void display_Steamline(Model_fftw* model_fft, int cell_size, Model_color* color);
	void bilinear_interpolation(int idx0, int idx1, int idx2, int idx3, double px0, double py0,
		double px1, double py1, double px2, double py2, double px3, double py3, double px, double py, double *p_velX,
		double* p_velY, Model_fftw* model_fft);
	void compute_velocity(double px, double py, double* p_velX, double* p_velY, Model_fftw* model_fft, int wn, int hn);
	void visualize(int DIM, Model_fftw* model_fft, Model_color* color,int* DENSITY, int* VELOCITY, int* FORCE, int* dataset,
		int* SCALAR_DENSITY, int* SCALAR_VELOCITY, int* SCALAR_FORCE, int* dataset_scalar,
		int* VECTOR_VELOCITY, int* VECTOR_FORCE, int* dataset_vector, float z, float alpha);
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
	int CONES;
	int ARROWS;
	int glyph_samplesX;
	int glyph_samplesY;
	int vector_type;
	int STANDARD;
	int GRADIENT;
	//Streamline parameters
	double MOUSEx, MOUSEy; //pixel value to draw the circle
	int GRIDx, GRIDy; //Grid value that we get by clicking  
	int draw_streamline;
	int streamline_size;
	int mouse_clicked;
	int streamline_finished;

	//slices parameters
	int draw_slices;
	float ex, ey, ez, cx, cy, cz, ux, uy, uz;
	float alpha;
	float z;

	//clamp_scale
	int bool_clamp_scale; //0 if clamp, 1 if scale
	
	

};

