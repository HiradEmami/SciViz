#include "View_visualization.h"
#include <rfftw.h>              //the numerical simulation Rlibrary
#include <GL/glut.h>            //the GLUT graphics library       
#include <iostream>				//for printing the help text
#include "Model_color.h"
#include "Model_fftw.h"
#include "Controller_keyboard.h"
#include <GL/glui.h>



View_visualization::View_visualization()
{
	 DIM = 3;
	 color_dir = 0;            //use direction color-coding or not
	 vec_scale = 1000;			//scaling of hedgehogs
	 draw_smoke = 1;           //draw the smoke or not
	 draw_vecs = 1;            //draw the vector field or not
	 colorbar_width = 50;
	 COLOR_BLACKWHITE = 0;
	 COLOR_GRAYSCALE = 1;
	 COLOR_RAINBOW = 2;
	 COLOR_HEATMAP = 3;
	 COLOR_DIVERGING = 4;
	 COLOR_TWOCOLORS = 5;
	 scalar_col = 0;
	 //default glyphs
	 glyph_type = 0;

}


//------ VISUALIZATION CODE STARTS HERE -----------------------------------------------------------------

void View_visualization::set_colormap(Model_color* color,float vy, int dataset)
{

	// change scale parameters depending on dataset
	if (dataset == 0 ) {
		color->max = color->density_max;
		color->min = color->density_min;
	}
	else if (dataset == 1) {
		color->max = color->vel_max;
		color->min = color->vel_min;
	}

	//color->density_max = color->vel_max;
	//color->density_min = color->vel_min;


	vy = color->clamp(vy);
	vy *= color->NCOLORS; vy = (int)vy; vy /= color->NCOLORS;
	float R, G, B;
	if (scalar_col == COLOR_BLACKWHITE)
		color->blackwhite(vy, &R, &G, &B);
	else if (scalar_col == COLOR_GRAYSCALE)
		color->grayscale(vy, &R, &G, &B);
	else if (scalar_col == COLOR_RAINBOW)
		color->rainbow(vy, &R, &G, &B);
	else if (scalar_col == COLOR_HEATMAP) {
		color->heatmap(vy, &R, &G, &B);
	}
	else if (scalar_col == COLOR_DIVERGING) {
		color->diverging(vy, &R, &G, &B);
	}
	else if (scalar_col == COLOR_TWOCOLORS) {
		color->interpolate(vy, &R, &G, &B, 0, 0, 1, 1, 1, 0);
	}
	float h, s, v;
	color->rgb2hsv(R, G, B, &h, &s, &v);
	color->hsv2rgb(h, s, v, &R, &G, &B);


	glColor3f(R, G, B);

}

//direction_to_color: Set the current color by mapping the magnitude of a direction vector (x,y), using
//the selected scalar dataset and colormap                    
void View_visualization::direction_to_color(float x, float y, float scalar, int colormap, Model_color color)
{
	scalar = color.clamp(scalar);
	float r, g, b, f;
	if (color_dir) {
		if (colormap == COLOR_BLACKWHITE)
		{
			// get orientation
			//f = atan2(y, x) / 3.1415927 + 1;

			/*r = f;
			if (r > 1) r = 2 - r;
			g = f + .66667;
			if (g > 2) g -= 2;
			if (g > 1) g = 2 - g;
			b = f + 2 * .66667;
			if (b > 2) b -= 2;
			if (b > 1) b = 2 - b;*/
			color.blackwhite(scalar, &r, &g, &b);
		}
		else if (colormap == COLOR_GRAYSCALE) {
			color.grayscale(scalar, &r, &g, &b);
		}
		else if (colormap == COLOR_RAINBOW) {
			color.rainbow(scalar, &r, &g, &b);
		}
		else if (colormap == COLOR_HEATMAP) {
			color.heatmap(scalar, &r, &g, &b);
		}
		else if (colormap == COLOR_DIVERGING) {
			color.diverging(scalar, &r, &g, &b);
		}
		else if (colormap == COLOR_TWOCOLORS) {
			color.interpolate(scalar, &r, &g, &b, 0, 0, 1, 1, 1, 0);
		}
	}
	else {
		r = g = b = 1;
	}
	glColor3f(r, g, b);
}

//compute the rgb values given the current segment and the current colormap
void View_visualization::compute_RGB(Model_color* color,float value, float* R, float* G, float* B) {
	value = color->clamp(value);
	switch (scalar_col) {
	case 0:
		color->blackwhite(value, R, G, B);
		break;
	case 1:
		color->grayscale(value, R, G, B);
		break;
	case 2:
		color->rainbow(value, R, G, B);
		break;
	case 3:
		color->heatmap(value, R, G, B);
		break;
	case 4:
		color->diverging(value, R, G, B);
		break;
	case 5:
		color->interpolate(value, R, G, B, 0, 0, 1, 1, 1, 0);
		break;
	}


}


//draw a colorbar with the currently selected colormap
void View_visualization::draw_colorbar(Model_color* color) {

	//the amount of 'strips'
	//int segments = 3;
	float R, G, B, value;
	//each 'strip' has the same height and width
	//float segment_height = winHeight / segments;
	float colorbar_height = winHeight / 10;

	glBegin(GL_QUAD_STRIP);

		// draw colorbar by using three sets of two vertices
		value = 0;
		compute_RGB(&*color, value, &R, &G, &B);
		float h, s, v;
		color->rgb2hsv(R, G, B, &h, &s, &v);
		color->hsv2rgb(h, s, v, &R, &G, &B);
		glColor3f(R, G, B);
		glVertex2f(0, 0);
		glVertex2f(0, colorbar_height);

		value = 0.5;
		compute_RGB(&*color, value, &R, &G, &B);
		color->rgb2hsv(R, G, B, &h, &s, &v);
		color->hsv2rgb(h, s, v, &R, &G, &B);
		glColor3f(R, G, B);
		glVertex2f(winWidth / 2 - winWidth / 8, 0);
		glVertex2f(winWidth / 2 - winWidth / 8, colorbar_height);

		value = 1.0;
		compute_RGB(&*color, value, &R, &G, &B);
		color->rgb2hsv(R, G, B, &h, &s, &v);
		color->hsv2rgb(h, s, v, &R, &G, &B);
		glColor3f(R, G, B);

		glVertex2f(winWidth - (winWidth / 5), 0);
		glVertex2f(winWidth - (winWidth / 5), colorbar_height);
	glEnd();

	draw_number(&*color, std::to_string(color->min), 5);
	draw_number(&*color, std::to_string((color->max + color->min) / 2), winWidth /2  - winWidth / 8);
	draw_number(&*color, std::to_string(color->max), winWidth - (winWidth / 5) - 20);


}



void View_visualization::draw_number(Model_color* color, std::string value, float position) {
	glColor3f(1, 1, 1);
	for (int i = 0; i < 3; i++) {
		glRasterPos2f(position + (5*i), winHeight / 10);
		//glRasterPos2f(i*5, position);
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, value[i]);
		
	}
	
	
	
}


void View_visualization::set_Glyph_type() {
	//glyphs parameters
	//glyph_line = 0;
	//glyph_line_loop = 1;
	//glyph_triangle =2;
	//glyph_triangle Fan = 3;
	//glyph_QUADS = 4;
	//glyph_quads_strip = 5;
	//glyph_POLYGON = 6;
	//glBegin(GL_LINES);

	switch (glyph_type)
	{
	case 0:
		glBegin(GL_LINES);
		break;
	case 1:
		glBegin(GL_LINE_LOOP);
		break;
	case 2:
		glBegin(GL_TRIANGLES);
		break;
	case 3:
		glBegin(GL_POINTS);
		break;
	case 4:
		glBegin(GL_QUADS);
		break;
	case 5:
		glBegin(GL_POLYGON);
		break;
	default:
		glBegin(GL_LINES);
		break;
	}

}


void View_visualization::visualize(int DIM, Model_fftw* model_fft,Model_color* color, int* DENSITY, int* VELOCITY, int* FORCE, int* dataset,
	int* SCALAR_DENSITY, int* SCALAR_VELOCITY, int* SCALAR_FORCE, int* dataset_scalar,
	int* VECTOR_VELOCITY, int* VECTOR_FORCE,int* dataset_vector)
{
	int        i, j, idx;
	fftw_real  wn = (fftw_real)winWidth / (fftw_real)(DIM + 1);   // Grid cell width
	fftw_real  hn = (fftw_real)winHeight / (fftw_real)(DIM + 1);  // Grid cell height
	float value0, value1, value2, value3;

	if (draw_smoke)
	{
		int idx0, idx1, idx2, idx3;
		double px0, py0, px1, py1, px2, py2, px3, py3;
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBegin(GL_TRIANGLES);
		for (j = 0; j < DIM - 1; j++)            //draw smoke
		{
			for (i = 0; i < DIM - 1; i++)
			{
				px0 = wn + (fftw_real)i * wn;
				py0 = hn + (fftw_real)j * hn;
				idx0 = (j * DIM) + i;


				px1 = wn + (fftw_real)i * wn;
				py1 = hn + (fftw_real)(j + 1) * hn;
				idx1 = ((j + 1) * DIM) + i;


				px2 = wn + (fftw_real)(i + 1) * wn;
				py2 = hn + (fftw_real)(j + 1) * hn;
				idx2 = ((j + 1) * DIM) + (i + 1);


				px3 = wn + (fftw_real)(i + 1) * wn;
				py3 = hn + (fftw_real)j * hn;
				idx3 = (j * DIM) + (i + 1);

				// draw smoke density
				if (*dataset == *DENSITY) {
					// scalar values are simply the rho/density values
					value0 = model_fft->rho[idx0];
					value1 = model_fft->rho[idx1];
					value2 = model_fft->rho[idx2];
					value3 = model_fft->rho[idx3];

				}
				// draw smoke velocity
				else if (*dataset == *VELOCITY) {
					// scalar values are the magnitudes of the vectors
					value0 = sqrt((model_fft->vx[idx0] * model_fft->vx[idx0]) + (model_fft->vy[idx0] * model_fft->vy[idx0]));
					value1 = sqrt((model_fft->vx[idx1] * model_fft->vx[idx1]) + (model_fft->vy[idx1] * model_fft->vy[idx1]));
					value2 = sqrt((model_fft->vx[idx2] * model_fft->vx[idx2]) + (model_fft->vy[idx2] * model_fft->vy[idx2]));
					value3 = sqrt((model_fft->vx[idx3] * model_fft->vx[idx3]) + (model_fft->vy[idx3] * model_fft->vy[idx3]));

					// increase velocity scalar to make it more visible
					value0 *= 10;
					value1 *= 10;
					value2 *= 10;
					value3 *= 10;

				}

				// draw smoke force field
				else if (*dataset == *FORCE) {
					value0 = sqrt((model_fft->fx[idx0] * model_fft->fx[idx0]) + (model_fft->fy[idx0] * model_fft->fy[idx0]));
					value1 = sqrt((model_fft->fx[idx1] * model_fft->fx[idx1]) + (model_fft->fy[idx1] * model_fft->fy[idx1]));
					value2 = sqrt((model_fft->fx[idx2] * model_fft->fx[idx2]) + (model_fft->fy[idx2] * model_fft->fy[idx2]));
					value3 = sqrt((model_fft->fx[idx3] * model_fft->fx[idx3]) + (model_fft->fy[idx3] * model_fft->fy[idx3]));

					// increase force scalar to make it more visible
					value0 *= 20;
					value1 *= 20;
					value2 *= 20;
					value3 *= 20;


				}

				set_colormap(&*color, value0, *dataset);    glVertex2f(px0, py0);
				set_colormap(&*color, value1, *dataset);    glVertex2f(px1, py1);
				set_colormap(&*color, value2, *dataset);    glVertex2f(px2, py2);

				set_colormap(&*color, value0, *dataset);    glVertex2f(px0, py0);
				set_colormap(&*color, value2, *dataset);    glVertex2f(px2, py2);
				set_colormap(&*color, value3, *dataset);    glVertex2f(px3, py3);
			}
		}
		glEnd();
	}

	//draw vector field by using glyphs
	float x, y, scalar, magnitude;
	if (draw_vecs)
	{
		//GL_TRIANGLES
		set_Glyph_type();
		for (i = 0; i < DIM; i++)
			for (j = 0; j < DIM; j++)
			{
				idx = (j * DIM) + i;
				// choose scalar for coloring 
				if (*dataset_scalar == *SCALAR_DENSITY) {
					scalar = model_fft->rho[idx];
				}
				else if (*dataset_scalar == *SCALAR_VELOCITY) {
					scalar = (model_fft->vx[idx] * model_fft->vx[idx]) + (model_fft->vy[idx] * model_fft->vy[idx]);
					scalar *= 100;
				}
				else if (*dataset_scalar == *SCALAR_FORCE) {
					scalar = (model_fft->fx[idx] * model_fft->fx[idx]) + (model_fft->fy[idx] * model_fft->fy[idx]);
					scalar *= 100;
				}
				//X and Y values depend on chosen dataset vector
				if (*dataset_vector == *VECTOR_VELOCITY) {
					x = model_fft->vx[idx];
					y = model_fft->vy[idx];
				}
				else if (*dataset_vector == *VECTOR_FORCE) {
					x = model_fft->fx[idx];
					y = model_fft->fy[idx];
				}
				// compute magnitude of chosen vector dataset
				magnitude = sqrt((x * x) + (y * y));
				magnitude *= 10;
				direction_to_color(x, y, scalar, scalar_col, *color);
				// use x and y to draw corresponding direction of vector
				glVertex2f(wn + (fftw_real)i * wn, hn + (fftw_real)j * hn);
				glVertex2f((wn + (fftw_real)i * wn) + (vec_scale + (magnitude * vec_scale)) * x, (hn + (fftw_real)j * hn) + (vec_scale + (magnitude * vec_scale)) * y);
				//glVertex2f((wn + (fftw_real)i * wn) + view.vec_scale  * x, (hn + (fftw_real)j * hn) + view.vec_scale * y);
			}
		glEnd();
	}


	//draw color bar
	draw_colorbar(&*color);




}


