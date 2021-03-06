#pragma once
#include <rfftw.h>              //the numerical simulation Rlibrary
#include <GL/glut.h>            //the GLUT graphics library       
#include <iostream>	
#include "Model_color.h"
#include "View_visualization.h"
#include "Model_fftw.h"


class Controller_keyboard
{
public:
	Controller_keyboard();
	void keyboard(View_visualization* view, unsigned char key, Model_color* color, Model_fftw* model_fft,  int* frozen, int* data, int* velocity);
};

