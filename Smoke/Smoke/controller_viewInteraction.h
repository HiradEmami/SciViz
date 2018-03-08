#pragma once
#include "Model_fftw.h"
#include "View_visualization.h"
class controller_viewInteraction
{
public:
	controller_viewInteraction();
	void drag(View_visualization* view, Model_fftw* model_fft, int DIM, int* mx, int* my);
	void reshape(View_visualization* view, int* w, int* h);
	void mouse(int* btn, int* state, int* x, int* y, View_visualization* view);

private:
	void setX(int x, View_visualization* view);
	void setY(int y, View_visualization* view);

};