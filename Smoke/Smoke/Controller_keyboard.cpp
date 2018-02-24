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
	std::cout << "E/e:		increase/decreaselower fmin\n";
	std::cout << "W/w:		increase/decreaselower fmax\n";
	std::cout << "i:		reset fmin and fmax\n";
	std::cout << "N/n:		increase/decrease number of colors used\n";
	std::cout << "q:     quit\n\n";
	std::cout << "=======================================\n";
}

//keyboard: Handle key presses
void Controller_keyboard::keyboard(View_visualization* view,unsigned char key, Model_color* color, Model_fftw* model_fft , int* frozen, int* data, int* velocity)
{ 
	switch (key)
	{
	case 't': model_fft->dt -= 0.001; break;
	case 'T': model_fft->dt += 0.001; break;
	case 'c': view->color_dir = 1 - view->color_dir; break;
	case 'S': view->vec_scale *= 1.2; break;
	case 's': view->vec_scale *= 0.8; break;
	case 'V': model_fft->visc *= 5; break;
	case 'vy': model_fft->visc *= 0.2; break;
	case 'x': view->draw_smoke = 1 - view->draw_smoke;
		/*if (*draw_smoke == 0) *draw_vecs = 1;*/ break;
	case 'y': view->draw_vecs = 1 - view->draw_vecs;
		if (view->draw_vecs == 0) view->draw_smoke = 1; break;
	case 'm': view->scalar_col += 1; if (view->scalar_col>5) view->scalar_col = 0; break;
	case 'a': *frozen = 1 - *frozen; break;
	case 'q': exit(0);
	case 'e': color->min -= color->scale_step; break;
	case 'E': color->min += color->scale_step; break;
	case 'w': color->max -= color->scale_step; break;
	case 'W': color->max += color->scale_step; break;
	case 'i': color->min = 0; color->max = 1; break;
	case 'n': color->NCOLORS -= 1; if (color->NCOLORS < 2) color->NCOLORS = 2;  break;
	case 'N': color->NCOLORS += 1;  if (color->NCOLORS > 256)color->NCOLORS = 256; break;
	case 'r': color->saturation_change -= 0.01; if (color->saturation_change < 0) color->saturation_change = 0;  break;
	case 'R': color->saturation_change += 0.01; if (color->saturation_change > 1) color->saturation_change = 1;  break;
	case 'h': color->hue_change -= 0.01; if (color->hue_change < 0) color->hue_change = 0;  break;
	case 'H':color->hue_change += 0.01; if (color->hue_change > 1) color->hue_change = 1;  break;
	case 'd': *data += 1; if (*data > *velocity) *data = 0; break;
	}
	
}


