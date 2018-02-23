#include "Controller_keyboard.h"
#include "Model_color.h"
#include "Model_fftw.h"
#include <GL/glut.h>
#include <rfftw.h>              //the numerical simulation Rlibrary
#include <GL/glut.h>            //the GLUT graphics library       
#include <iostream>	
#include "View_visualization.h"




Controller_keyboard::Controller_keyboard()
{
	std::cout << "Fluid Flow Simulation and Visualization\n";
	std::cout << "=======================================\n";
	std::cout << "Click and drag the mouse to steer the flow!\n";
	std::cout << "T/t:   increase/decrease simulation timestep\n";
	std::cout << "S/s:   increase/decrease hedgehog scaling\n";
	std::cout << "c:     toggle direction coloring on/off\n";
	std::cout << "V/vy:   increase decrease fluid viscosity\n";
	std::cout << "x:     toggle drawing matter on/off\n";
	std::cout << "y:     toggle drawing hedgehogs on/off\n";
	std::cout << "m:     toggle thru scalar coloring\n";
	std::cout << "a:     toggle the animation on/off\n";
	std::cout << "4:		lower fmin\n";
	std::cout << "6:		increase fmin\n";
	std::cout << "2:		lower fmax\n";
	std::cout << "8:		increase fmax\n";
	std::cout << "9:		reset fmin and fmax\n";
	std::cout << "n:		decrease number of colors used\n";
	std::cout << "N:		increase number of colors used\n";
	std::cout << "q:     quit\n\n";
}

//keyboard: Handle key presses
void Controller_keyboard::keyboard(int *scalar_col,int *draw_vecs,int* draw_smoke,float* vec_scale,int* color_dir,unsigned char key, Model_color* color, Model_fftw* model_fft , int* frozen, int* data, int* velocity)
{ 
	switch (key)
	{
	case 't': model_fft->dt -= 0.001; break;
	case 'T': model_fft->dt += 0.001; break;
	case 'c': *color_dir = 1 - *color_dir; break;
	case 'S': *vec_scale *= 1.2; break;
	case 's': *vec_scale *= 0.8; break;
	case 'V': model_fft->visc *= 5; break;
	case 'vy': model_fft->visc *= 0.2; break;
	case 'x': *draw_smoke = 1 - *draw_smoke;
		/*if (*draw_smoke == 0) *draw_vecs = 1;*/ break;
	case 'y': *draw_vecs = 1 - *draw_vecs;
		if (*draw_vecs == 0) *draw_smoke = 1; break;
	case 'm': *scalar_col += 1; if (*scalar_col>5) *scalar_col = 0; break;
	case 'a': *frozen = 1 - *frozen; break;
	case 'q': exit(0);
	case '4': color->scale_min -= color->scale_step; break;
	case '6': color->scale_min += color->scale_step; break;
	case '2': color->scale_max -= color->scale_step; break;
	case '8': color->scale_max += color->scale_step; break;
	case '9': color->scale_min = 0; color->scale_max = 1; break;
	case 'n': color->NCOLORS -= 1; if (color->NCOLORS < 2) color->NCOLORS = 2;  break;
	case 'N': color->NCOLORS += 1;  if (color->NCOLORS > 256)color->NCOLORS = 256; break;
	case 'r': color->saturation_change -= 0.01; if (color->saturation_change < 0) color->saturation_change = 0;  break;
	case 'R': color->saturation_change += 0.01; if (color->saturation_change > 1) color->saturation_change = 1;  break;
	case 'h': color->hue_change -= 0.01; if (color->hue_change < 0) color->hue_change = 0;  break;
	case 'H':color->hue_change += 0.01; if (color->hue_change > 1) color->hue_change = 1;  break;
	case 'd': *data += 1; if (*data > *velocity) *data = 0; break;
	}
	
}


