#pragma once
#include "Model_fftw.h"
#include "View_visualization.h"
class controller_viewInteraction
{
public:
	controller_viewInteraction();
	void drag(View_visualization* view, Model_fftw* model_fft, int DIM, int* mx, int* my);
	void reshape(View_visualization* view, int* w, int* h);
};