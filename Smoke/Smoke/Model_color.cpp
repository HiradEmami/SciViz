#include "Model_color.h"
#include <GL/glut.h> 
#include <math.h>

Model_color::Model_color()
{
	hue_change = 1;
	saturation_change = 1;
	NCOLORS = 255;
	scale_step = 0.01;
	scale_max = 1.0;
	scale_min = 0.0;
	test = 0;

}
void Model_color::rgb2hsv(float r, float g, float b, float *h, float *s, float *v)
{
	float M = fmax(r, fmax(g, b));
	float m = fmin(r, fmin(g, b));
	float d = M - m;
	*v = M; //value = max( r ,g ,b)
	*s = (M>0.00001) ? d / M : 0; //saturation
	if (s == 0) h = 0; //achromatic case , hue=0 by convention
	else //chromatic case
	{
		if (r == M) *h = (g - b) / d;
		else if (g == M) *h = 2 + (b - r) / d;
		else *h = 4 + (r - g) / d;
		*h /= 6;
		if (h<0) h += 1;
	}

	// user controlled h and s
	*s *= saturation_change;
	*h *= hue_change;
}

void Model_color::hsv2rgb(float h, float s, float v, float *r, float *g, float *b) {
	int hueCase = (int)(h * 6);
	float frac = 6 * h - hueCase;
	float lx = v *(1 - s);
	float ly = v*(1 - s * frac);
	float lz = v*(1 - s *(1 - frac));
	switch (hueCase)
	{
	case 0:
	case 6: *r = v; *g = lz; *b = lx; break; // 0<hue<1/6
	case 1: *r = ly; *g = v; *b = lx; break; // 1/6<hue<2/6
	case 2: *r = lx; *g = v; *b = lz; break; // 2/6<hue<3/6
	case 3: *r = lx; *g = ly; *b = v; break; // 3/6<hue/4/6
	case 4: *r = lz; *g = lx; *b = v; break; // 4/6<hue<5/6
	case 5: *r = v; *g = lx; *b = ly; break; // 5/6<hue<1
	}
}

void Model_color::interpolate(float value, float* R, float* G, float* B, float r1, float g1, float b1, float r2, float g2, float b2)
{
	*R = r1 * (1.0 - value) + value * r2;
	*G = g1 * (1.0 - value) + value * g2;
	*B = b1 * (1.0 - value) + value * b2;
}

//rainbow: Implements a color palette, mapping the scalar 'value' to a rainbow color RGB
void Model_color::rainbow(float value, float* R, float* G, float* B)
{
	const float dx = 0.8;
	if (value<0) value = 0; if (value>1) value = 1;
	value = (6 - 2 * dx)*value + dx;
	*R = fmax(0.0, (3 - fabs(value - 4) - fabs(value - 5)));
	*G = fmax(0.0, (4 - fabs(value - 2) - fabs(value - 4)));
	*B = fmax(0.0, (3 - fabs(value - 1) - fabs(value - 2)));
}
void Model_color::grayscale(float value, float* R, float* G, float* B)
{
	if (value<0) value = 0; if (value>1) value = 1;
	value = value / 3;
	*R = *G = *B = value;
}

void Model_color::heatmap(float value, float* R, float* G, float* B)
{

	float r1, g1, b1, r2, g2, b2;
	if (value<scale_min) value = scale_min; if (value>scale_max) value = scale_max;
	//orange 
	r2 = 0.9*value;
	g2 = 0;
	b2 = 0;

	float mid = (scale_max + scale_min) / 2;

	if (value <= mid) {
		//black
		r1 = g1 = b1 = 0;

		//interpolate between black and orange
		interpolate(value * 2, R, G, B, r1, g1, b1, r2, g2, b2);
	}
	else {
		//white
		r1 = g1 = 1;
		b1 = 0.1*value;
		//interpolate between orange and white
		interpolate((value - 0.5) * 2, R, G, B, r2, g2, b2, r1, g1, b1);
	}


	/**R = value + 0.2;
	*G = value * (value/1.2);
	*B = 0;*/
}

void Model_color::blackwhite(float value, float* R, float* G, float* B)
{
	*R = *G = *B = value;
}




void Model_color::diverging(float value, float* R, float* G, float* B)
{
	float r1, g1, b1, r2, g2, b2;
	//white 
	r2 = g2 = b2 = 1;
	if (value<scale_min) value = scale_min; if (value>scale_max) value = scale_max;
	float mid = (scale_max + scale_min) / 2;

	if (value <= mid) {
		//blue
		r1 = g1 = 0;
		b1 = 0.9;
		//interpolate between green and white
		interpolate(value * 2, R, G, B, r1, g1, b1, r2, g2, b2);
	}
	else {
		//red
		g1 = b1 = 0;
		r1 = 1;
		//interpolate between white and red
		interpolate((value - 0.5) * 2, R, G, B, r2, g2, b2, r1, g1, b1);
	}

}
//set_colormap: Sets different types of colormaps



