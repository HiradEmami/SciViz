#pragma once
#include <math.h>
class Model_color
{
public:
	Model_color();
	// parameters for scaling and clamping
	float scale_min;
	float scale_max;
	float scale_step;
	int NCOLORS;
	float saturation_change;
	float hue_change;

	int test;
	//functions

	void diverging(float value, float* R, float* G, float* B);
	void blackwhite(float value, float* R, float* G, float* B);
	void heatmap(float value, float* R, float* G, float* B);
	void grayscale(float value, float* R, float* G, float* B);
	void rainbow(float value, float* R, float* G, float* B);
	void interpolate(float value, float* R, float* G, float* B, float r1, float g1, float b1, float r2, float g2, float b2);
	void hsv2rgb(float h, float s, float v, float *r, float *g, float *b);
	void rgb2hsv(float r, float g, float b, float *h, float *s, float *v);
};
