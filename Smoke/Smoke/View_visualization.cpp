#include "View_visualization.h"
#include <rfftw.h>              //the numerical simulation Rlibrary
#include <GL/glut.h>            //the GLUT graphics library       
#include <iostream>				//for printing the help text
#include "Model_color.h"
#include "Model_fftw.h"
#include "Controller_keyboard.h"
#include <GL/glui.h>
#define _USE_MATH_DEFINES
#include <math.h>

View_visualization::View_visualization() {

}

View_visualization::View_visualization(float width, float height)
{
	winWidth = width;
	winHeight = height;

	 DIM = 50;
	 color_dir = 0;            //use direction color-coding or not
	 vec_scale = 1000;			//scaling of hedgehogs
	 draw_smoke = 1;           //draw the smoke or not
	 draw_vecs = 1;            //draw the vector field or not
	 use_clamp = 1;
	 colorbar_width = 50;
	 COLOR_BLACKWHITE = 0;
	 COLOR_GRAYSCALE = 1;
	 COLOR_RAINBOW = 2;
	 COLOR_HEATMAP = 3;
	 COLOR_DIVERGING = 4;
	 COLOR_TWOCOLORS = 5;
	 scalar_col = 2;
	
	 data_min = 100;
	 data_max = 0;
	 //glyph parameters
	 glyph_type = 2;
	 LINES = 0;
	 CONES = 1;
	 ARROWS = 2;

	 //draw normal vector glyphs or gradient glyphs
	 vector_type = 0;
	 VELOCITY_FIELD = 0;
	 FORCE_FIELD = 1;
	 DENSITY_GRADIENT_FIELD = 2;
	 VELOCITY_GRADIENT_FIELD = 3;
	 


	 glyph_samplesX = 50;
	 glyph_samplesY = 50;

	 //streamline parameters
	 draw_streamline = 0; 
	 MOUSEx = 0;
	 MOUSEy = 0;
	 GRIDx = 0;
	 GRIDy = 0;
	 streamline_size = 120;
	 mouse_clicked = 0;
	 streamline_finished = 1;

	 //slices parameters
	 draw_slices = 0;
	 ex = 0;  ey = 0;  ez = 0;
	 cx = 0;  cy = 0;  cz = -1;
	 ux = 0;  uy = 1;  uz = 0;
	 //clamp_scale
	 //bool_clamp_scale = 0;
	 
	
}

//------ VISUALIZATION CODE STARTS HERE -----------------------------------------------------------------

void View_visualization::set_colormap(Model_color* color, float vy, int dataset)
{
	// change scale parameters depending on dataset
	if (dataset == 0) {
		color->max = color->density_max;
		color->min = color->density_min;
	}
	else if (dataset == 1) {
		color->max = color->vel_max;
		color->min = color->vel_min;
	}

	//color->density_max = color->vel_max;
	//color->density_min = color->vel_min;

	if (use_clamp) {
		vy = color->clamp(vy);
	}
	else {
		vy = color->scale(vy, data_min, data_max);
	}
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

	glColor4f(R, G, B, alpha);

}
//direction_to_color: Set the current color by mapping the magnitude of a direction vector (x,y), using
//the selected scalar dataset and colormap                    
void View_visualization::direction_to_color(float scalar, int colormap, Model_color color)
{

	float r, g, b, f;
	if (color_dir) {
		if (colormap == COLOR_BLACKWHITE)
		{
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
	glColor4f(r, g, b, alpha);
}
//compute the rgb values given the current segment and the current colormap
void View_visualization::compute_RGB(Model_color* color,float value, float* R, float* G, float* B) {
	if (use_clamp) {
		value = color->clamp(value);
	}
	else {
		value = color->scale(value, data_min, data_max);
	}
	

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
void View_visualization::draw_colorbar(Model_color* color) {
    // we use NCOLORS amount of rectangles in the colorbar
	float R, G, B, H, S, V;
	//each piece has the same height and width
	float colorbar_width = winWidth - (winWidth / 5);
	float segment_width = colorbar_width / (color->NCOLORS + 1);
	float segment_value = 0;
	float colorbar_height = winHeight / 20;

	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= color->NCOLORS; i++) {
		segment_value = (float)i / color->NCOLORS;
		compute_RGB(&*color, segment_value, &R, &G, &B);
		color->rgb2hsv(R, G, B, &H, &S, &V);
		color->hsv2rgb(H, S, V, &R, &G, &B);
		glColor3f(R, G, B);
		glVertex3f((i * segment_width), 0, z);
		glVertex3f((i * segment_width), colorbar_height, z);

		glVertex3f((i * segment_width) + segment_width, 0, z);
		glVertex3f((i * segment_width) + segment_width, colorbar_height, z);

	}
	glEnd();
	
	float min, mid, max,firsthalf, secondhalf;
	if (use_clamp) {
		min = color->min;
		max = color->max;
		mid = 0.5*(color->max + color->min);
		firsthalf = mid - min / 2;
		secondhalf = max - mid / 2;
	}
	else {
		min = data_min;
		max = data_max;
		mid = 0.5*(data_min + data_max);
		firsthalf = mid - min / 2;
		secondhalf = max - mid / 2;
	
	}

	draw_number(&*color, std::to_string(min), 5, colorbar_height + 5);
	draw_number(&*color, std::to_string(firsthalf), (winWidth / 2 - winWidth / 8)/2, colorbar_height + 5);
	draw_number(&*color, std::to_string(mid), winWidth / 2 - winWidth / 8, colorbar_height + 5);
	draw_number(&*color, std::to_string(secondhalf),(winWidth / 2 - winWidth / 8)+(((winWidth - (winWidth / 5) - 20) - (winWidth / 2 - winWidth / 8)) / 2)  ,colorbar_height + 5);
	draw_number(&*color, std::to_string(max), winWidth - (winWidth / 5) - 20, colorbar_height + 5);
}
void View_visualization::draw_number(Model_color* color, std::string value, float position, float height) {
	glColor3f(1, 1, 1);
	for (int i = 0; i < 5; i++) {
		glRasterPos3f(position + (7*i), height, z);
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
		break;
	case 1:
		break;
	}
}
void View_visualization::drawCircle(GLfloat cx, GLfloat cy, GLfloat radius) {
	float i = 0.0f;

	glBegin(GL_TRIANGLE_FAN);

	glVertex2f(cx, cy); // Center
	for (i = 0.0f; i <= 360; i++)
		glVertex2f(radius*cos(M_PI * i / 180.0) + cx, radius*sin(M_PI * i / 180.0) + cy);

	glEnd();

}
void View_visualization::drawLine(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
	glLineWidth(5);
	//glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(x1, y1, z);
	glVertex3f(x2, y2, z);
	glEnd();
}

// apply the coordinate transform to get the reference cell coordinates for the given point p
void View_visualization::get_reference_coordinates(double px, double py, double v1x, double v1y, double v2x, double v2y,
	double v4x, double v4y, double* r, double* s) {

	*r = (((px - v1x) * (v2x - v1x)) + ((py - v1y) * (v2y - v1y))) / ((v2x - v1x) * (v2x - v1x) + (v2y - v1y) *  (v2y - v1y));
	*s = (((px - v1x) * (v4x - v1x)) + ((py - v1y) * (v4y - v1y))) / ((v4x - v1x) * (v4x - v1x) + (v4y - v1y) *  (v4y - v1y));
	
}

void View_visualization::draw_cones(float x, float y, fftw_real  wn, fftw_real hn, int i, int j, float magnitude) {
	
	float angle;
	// define the radius to be half of the cell width
	float radius = wn;
	glPushMatrix();
	// Translate glyph to middle of current cell
	glTranslatef(wn + (fftw_real)i * wn, hn + (fftw_real)j * hn, z);
	// Compute arctangent of vector y and x values
	angle = atan2(y, x);
	angle = 180 * angle / M_PI;
	// Rotate glyph according to computed vector orientation
	glRotatef(90 - angle, 0.0, 0.0, -1.0);
	glRotatef(-90.0, 1.0, 0.0, 0.0);
	// Draw the solid cones with specified radius
	glutSolidCone(radius, radius, 30, 3);
	glPopMatrix();
}

void View_visualization::rotate(float x, float y, float* newx, float* newy, float pivotx, float pivoty, float angle) {
	x = x - pivotx;
	y = y - pivoty;

	float c, s;
	c = cos(angle);
	s = sin(angle);

	*newx = (x * c) - (y * s);
	*newy = (x * s) + (y * c);
	*newx = *newx + pivotx;
	*newy = *newy + pivoty;
}

void View_visualization::draw_arrows(float x, float y, fftw_real  wn, fftw_real hn, float i, float j, float magnitude) {
	
	float base_scaling = 200;
	//glPushMatrix();
	//zglTranslatef(0, 400.0f, 0);
	
	float base_x, base_y, tip_x, tip_y;

	float max_length_x = wn * 1.5;
	float max_length_y = hn * 1.5;
	
	tip_x = vec_scale * x;
	tip_y = vec_scale * y;

	if (tip_x > max_length_x) {
		tip_x = max_length_x;
		base_scaling /= 2;
	}
	if (tip_x < -1 * max_length_x) {
		tip_x = -1 * max_length_x;
		base_scaling /= 2;
	}

	if (tip_y > max_length_y) {
		tip_y = max_length_y;
		base_scaling /= 2;
	}
	if (tip_y < -1 * max_length_y) {
		tip_y = -1 * max_length_y;
		base_scaling /= 2;
	}


	base_x = base_scaling * x;
	base_y = base_scaling * y;




	glBegin(GL_TRIANGLES);
		glVertex3f(i - base_x, j + base_y, z);
		glVertex3f(i + base_x, j - base_y, z);
		glVertex3f(i + tip_x, j + tip_y, z);
	glEnd();
	//glPopMatrix();



}

void View_visualization::draw_hedgehogs(float x, float y, fftw_real  wn, fftw_real hn, float i, float j, float magnitude) {
	// use x and y to draw corresponding direction of vector
	glLineWidth(5);
	glBegin(GL_LINES);
	glVertex3f(wn + (fftw_real)i * wn, hn + (fftw_real)j * hn, z);
	glVertex3f((wn + (fftw_real)i * wn) + (vec_scale * x), (hn + (fftw_real)j * hn) + (vec_scale * y), z);
	glEnd();
}


void View_visualization::visualize(int DIM, Model_fftw* model_fft,Model_color* color,int* DENSITY, int* VELOCITY, int* FORCE, int* dataset,
	int* SCALAR_DENSITY, int* SCALAR_VELOCITY, int* SCALAR_FORCE, int* dataset_scalar,
	float slicedepth, float a, float shift)

{
	alpha = a;
	z = slicedepth;

	
	//printf("%g\n", cz);

	if (draw_slices) {
		glShadeModel(GL_SMOOTH);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(ex, ey, ez, cx, cy, cz, ux, uy, uz);
		//glLoadIdentity();
		glPushMatrix();
		glTranslatef(shift, shift, 0.0f);
		//glMatrixMode(GL_MODELVIEW);
		//glLoadIdentity();
		//gluLookAt(ex, ey, ez, cx, cy, cz, ux, uy, uz);
		
	}
	



	int        i, j, idx;
	fftw_real  wn = (fftw_real)winWidth / (fftw_real)(DIM + 1);   // Grid cell width
	fftw_real  hn = (fftw_real)winHeight / (fftw_real)(DIM + 1);  // Grid cell height
	float value0, value1, value2, value3;
	int idx0, idx1, idx2, idx3;
	double px0, py0, px1, py1, px2, py2, px3, py3;
	double r, s;
	float value;

	if (draw_smoke)
	{
		data_min = 100;
		data_max = 0;
		if (!use_clamp) {
			//reset min and max values
			// compute min and max of selected dataset
			for (i = 0; i < DIM * 2 * (DIM / 2 + 1); i++) {
				if (*dataset == *DENSITY) {
					if (model_fft->rho[i] < data_min) {
						data_min = model_fft->rho[i];
					
					}
					if (model_fft->rho[i] > data_max) {
						data_max = model_fft->rho[i];
					}
				}
				else if (*dataset == *VELOCITY) {
					value = sqrt((model_fft->vx[i] * model_fft->vx[i]) + (model_fft->vy[i] * model_fft->vy[i]));
					value *= 10;
					
					if (value < data_min) {
						data_min = value;
					}
					if (value > data_max) {
						data_max = value;
					}
				}
				else {
					value = sqrt((model_fft->fx[i] * model_fft->fx[i]) + (model_fft->fy[i] * model_fft->fy[i]));
					value *= 20;
					if (value < data_min) {
						data_min = value;
					}
					if (value > data_max) {
						data_max = value;
					}
				}
				if (data_max > 1) data_max = 1;
				if (data_max < 0.01) data_max = 0;
				if (data_min < 0) data_min = 0;

				

			}
		}
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

				// Draw the smoke at all four vertices, by drawing two triangles

		
				set_colormap(&*color, value0, *dataset);    glVertex3f(px0, py0, z);
				set_colormap(&*color, value1, *dataset);    glVertex3f(px1, py1, z);
				set_colormap(&*color, value2, *dataset);    glVertex3f(px2, py2, z);

				set_colormap(&*color, value0, *dataset);    glVertex3f(px0, py0, z);
				set_colormap(&*color, value2, *dataset);    glVertex3f(px2, py2, z);
				set_colormap(&*color, value3, *dataset);    glVertex3f(px3, py3, z);

				
			}
		}
		glEnd();
	}
	
	
	wn = (fftw_real)winWidth / (fftw_real)(glyph_samplesX + 1);   // Grid cell width
	hn = (fftw_real)winHeight / (fftw_real)(glyph_samplesY + 1);  // Grid cell height
	//draw vector field by using glyphs
	double x, y;
	float scalar, magnitude;
	if (draw_vecs)
	{
		for (j = 0; j < glyph_samplesY; j += 1)
		{
			for (i = 0; i < glyph_samplesX; i += 1)
			{
				idx = (j * DIM) + i;
				// choose scalar for coloring 
				if (*dataset_scalar == *SCALAR_DENSITY) {
					scalar = model_fft->rho[idx];
				}
				else if (*dataset_scalar == *SCALAR_VELOCITY) {
					scalar = (model_fft->vx[idx] * model_fft->vx[idx]) + (model_fft->vy[idx] * model_fft->vy[idx]);
					scalar *= 1000;
				}
				else if (*dataset_scalar == *SCALAR_FORCE) {
					scalar = (model_fft->fx[idx] * model_fft->fx[idx]) + (model_fft->fy[idx] * model_fft->fy[idx]);
					scalar *= 1000;
				}


				idx0 = (j * DIM) + i;
				idx1 = ((j + 1) * DIM) + i;
				idx2 = ((j + 1) * DIM) + (i + 1);
				idx3 = (j * DIM) + (i + 1);

				px0 = wn + (fftw_real)i * wn;
				py0 = hn + (fftw_real)j * hn;


				px1 = wn + (fftw_real)i * wn;
				py1 = hn + (fftw_real)(j + 1) * hn;


				px2 = wn + (fftw_real)(i + 1) * wn;
				py2 = hn + (fftw_real)(j + 1) * hn;


				px3 = wn + (fftw_real)(i + 1) * wn;
				py3 = hn + (fftw_real)j * hn;

				get_reference_coordinates(wn + (fftw_real)i * wn, hn + (fftw_real)j * hn, px0, py0,
					px1, py1, px3, py3, &r, &s);



				if (vector_type == VELOCITY_FIELD) {
						x = model_fft->vx[idx];
						y = model_fft->vy[idx];
				}

				else if (vector_type == FORCE_FIELD) {
					x = model_fft->fx[idx];
					y = model_fft->fy[idx];
				}
		

				// draw the gradient of either density or velocity magnitude
				else if (vector_type == DENSITY_GRADIENT_FIELD) {

					y = (1 - s)*((model_fft->rho[idx1] - model_fft->rho[idx0]) / wn) + s * ((model_fft->rho[idx2] - model_fft->rho[idx3]) / wn);
					x = (1 - r)*((model_fft->rho[idx3] - model_fft->rho[idx0]) / hn) + r * ((model_fft->rho[idx2] - model_fft->rho[idx1]) / hn);		
				}
				else if (vector_type == VELOCITY_GRADIENT_FIELD) {

					value0 = sqrt((model_fft->vx[idx0] * model_fft->vx[idx0]) + (model_fft->vy[idx0] * model_fft->vy[idx0]));
					value1 = sqrt((model_fft->vx[idx1] * model_fft->vx[idx1]) + (model_fft->vy[idx1] * model_fft->vy[idx1]));
					value2 = sqrt((model_fft->vx[idx2] * model_fft->vx[idx2]) + (model_fft->vy[idx2] * model_fft->vy[idx2]));
					value3 = sqrt((model_fft->vx[idx3] * model_fft->vx[idx3]) + (model_fft->vy[idx3] * model_fft->vy[idx3]));

					y = (1 - s)*((value1 - value0) / wn) + s * ((value2 - value3) / wn);
					x = (1 - r)*((value3 - value0) / hn) + r * ((value2 - value1) / hn);

					y *= 100;
					x *= 100;
				}

				// compute magnitude of chosen vector dataset
				magnitude = sqrt((x * x) + (y * y));
				magnitude *= 10;


				//standard glyph position
				float posx, posy;
				posx = wn + (fftw_real)i * wn;
				posy = hn + (fftw_real)j * hn;
			
				// find grid cell that current glyph falls into
				float gridx = (int)model_fft->clamp((double)(DIM + 1) * ((double)posx / (double)winWidth));
				float gridy = (int)model_fft->clamp((double)(DIM + 1) * ((double)(posy) / (double)winHeight));
				
				// use nearest neigbor interpolation, take vx and vy of nearest grid point
				if (vector_type == VELOCITY_FIELD || vector_type == FORCE_FIELD) {
					x = model_fft->vx[(int)((gridy * DIM) + gridx)];
					y = model_fft->vy[(int)((gridy * DIM) + gridx)];
				}
				
			

				// Color the glyphs
				direction_to_color(scalar, scalar_col, *color);

				if (glyph_type == LINES) {
					draw_hedgehogs(x, y, wn, hn, posx, posy, magnitude);
				}
				else if (glyph_type == CONES) {
					draw_cones(x, y, wn, hn, posx, posy, magnitude);
				}
				else if (glyph_type == ARROWS) {
					draw_arrows(x, y, wn, hn, posx, posy, magnitude);
				}
			}
		}
	}
	
	//glDisable(GL_LIGHTING);
	//draw color bar

	if (!draw_slices) {
		draw_colorbar(color);
	}

	
	if (draw_streamline == 1) {
		display_Steamline(&*model_fft, wn, color);
		//drawCircle(MOUSEx, MOUSEy, 5);
	}

	glPopMatrix();




	/*int xi, yi, X, Y; double  dx, dy, len;
	static int lmx = 0, lmy = 0;				//remembers last mouse location

	xi = yi = 25;

	X = xi; Y = yi;

	if (X > (DIM - 1))  X = DIM - 1; if (Y > (DIM - 1))  Y = DIM - 1;
	if (X < 0) X = 0; if (Y < 0) Y = 0;

	int mx, my;
	mx = 672;
	my = 422;
	// Add force at the cursor location 
	my = winHeight - my;
	dx = mx - lmx; dy = my - lmy;
	len = sqrt(dx * dx + dy * dy);
	if (len != 0.0) { dx *= 0.1 / len; dy *= 0.1 / len; }
	model_fft->fx[Y * DIM + X] += dx;
	model_fft->fy[Y * DIM + X] += dy;
	model_fft->rho[Y * DIM + X] = 10.0f;
	lmx = mx; lmy = my;


	/*model_fft->fx[25] += 2;
	model_fft->fy[25] += 2;
	model_fft->rho[25] = 20.0f;*/
	
}

void View_visualization::bilinear_interpolation(int idx0, int idx1, int idx2, int idx3, double px0, double py0,
	double px1, double py1, double px2, double py2, double px3, double py3, double px, double py, double *interpolation_x,
	double* interpolation_y, Model_fftw* model_fft) {

	double valuex0, valuex1, valuex2, valuex3;
	double valuey0, valuey1, valuey2, valuey3;

	double xDiff = px2 - px0;
	double ydiff = py2 - py0;

	//x velocity values of cell vertices
	valuex0 = model_fft->vx[idx0];
	valuex1 = model_fft->vx[idx1];
	valuex2 = model_fft->vx[idx2];
	valuex3 = model_fft->vx[idx3];

	//y velocity values of cell vertices
	valuey0 = model_fft->vy[idx0];
	valuey1 = model_fft->vy[idx1];
	valuey2 = model_fft->vy[idx2];
	valuey3 = model_fft->vy[idx3];

	double x1, x2, y1, y2;

	//printf("Vertex values X: %g %g %g %g\n", valuex0, valuex1, valuex2, valuex3);
	//printf("Vertex values Y: %g %g %g %g\n", valuey0, valuey1, valuey2, valuey3);

	x1 = ((px2 - px) / xDiff) * valuex0 + ((px - px0) / xDiff) * valuex1;
	x2 = ((px2 - px) / xDiff) * valuex2 + ((px - px0) / xDiff) * valuex3;
	*interpolation_x = ((py1 - py) / ydiff) * x1 + ((py - py0) / ydiff) * x2;



	y1 = ((px2 - px) / xDiff) * valuey0 + ((px - px0) / xDiff) * valuey1;
	y2 = ((px2 - px) / xDiff) * valuey2 + ((px - px0) / xDiff) * valuey3;
	*interpolation_y = ((py1 - py) / ydiff) * y1 + ((py - py0) / ydiff) * y2;

	//printf("Velocity: %g %g\n", *interpolation_x, *interpolation_y);

}
void View_visualization::compute_velocity(double px, double py, double* p_velX, double* p_velY, Model_fftw* model_fft, int wn, int hn) {
	int idx0, idx1, idx2, idx3;
	double px0, py0, px1, py1, px2, py2, px3, py3;
	int i, j;
	
	// The grid position of point p (px, py) is already computed
	//printf("VEL: Gridx: %d Gridy: %d\n", GRIDx, GRIDy);
	i = GRIDx;
	j = GRIDy;


	// Compute indices of grid points and their coordinates
	idx0 = (i * DIM) + j;
	idx1 = ((i + 1) * DIM) + j;
	idx2 = ((i + 1) * DIM) + (j + 1);
	idx3 = (i * DIM) + (j + 1);

	//printf("Indices: %d %d %d %d\n", idx0, idx1, idx2, idx3);

	// lower left corner
	px0 = wn + (fftw_real)i * wn;
	py0 = hn + (fftw_real)j * hn;

	// lower right corner
	px1 = wn + (fftw_real)i * wn;
	py1 = hn + (fftw_real)(j + 1) * hn;

	// upper right corner
	px2 = wn + (fftw_real)(i + 1) * wn;
	py2 = hn + (fftw_real)(j + 1) * hn;

	//upper left corner
	px3 = wn + (fftw_real)(i + 1) * wn;
	py3 = hn + (fftw_real)j * hn;

	//printf("X: %g %g %g %g\n", px0, px1, px2, px3);
	//printf("Y: %g %g %g %g\n", py0, py1, py2, py3);
    //compute x and y velocity by interpolating by the four vertices of the cell
	bilinear_interpolation(idx0, idx1, idx2, idx3, px0, py0,
		px1, py1, px2, py2, px3, py3, px, py, p_velX,
		p_velY, model_fft);
	


}

void View_visualization::display_Steamline(Model_fftw* model_fft, int cell_size, Model_color* color) {
	if (mouse_clicked) {
		cell_size = cell_size / 2;
		//Draw the cicle for the first point
		//drawCircle(MOUSEx, MOUSEy, 5);
		//Calculating the other points
		double VELOCITYx, VELOCITYy; //speed
		VELOCITYx = 1;
		VELOCITYy = 1;
		double CURRENTx, CURRENTy; //the starting point of the line
		double start_point_X, start_point_Y; //keep track of the starting point
		int start_grid_X, start_grid_Y; //keep track of the starting grid
		start_point_X = MOUSEx;
		start_point_Y = MOUSEy;
		start_grid_X = GRIDx;
		start_grid_Y = GRIDy; 

		//printf("StartX: %g StartY: %g GRIDx: %d GRIDy: %d\n", start_point_X, start_point_Y, start_grid_X, start_grid_Y);

		float R, G, B;
		double magnitude;
		int i = 0;
		CURRENTx = MOUSEx;
		CURRENTy = MOUSEy;

		
		
		while(i < streamline_size && CURRENTx > 0 && CURRENTx < winWidth && CURRENTy > 0 && CURRENTy < winHeight)
		{
			//taking the current x and y
			CURRENTx = MOUSEx;
			CURRENTy = MOUSEy;
			//do the operation to calculate the next point and return VELOCITYx and VELOCITYy
			//compute_velocity(CURRENTx, CURRENTy, &VELOCITYx, &VELOCITYy, &*model_fft, cell_size, cell_size);

			// nearest neighbour interpolation
			VELOCITYx = model_fft->vx[(GRIDy * DIM) + GRIDx];
			VELOCITYy = model_fft->vy[(GRIDy * DIM) + GRIDx];
			//normalize velocity
			magnitude = sqrt((VELOCITYx * VELOCITYx) + (VELOCITYy * VELOCITYy));

			//stop drawing when velocity is zero!
			if (VELOCITYx == 0 && VELOCITYy == 0) {
				break;
			}

			VELOCITYx = VELOCITYx / magnitude;
			VELOCITYy = VELOCITYy / magnitude;


			magnitude *= 100;
			//color streamline using velocity magnitude
			compute_RGB(color, magnitude, &R, &G, &B);
			glColor3f(R, G, B);
			
	
		
			//calculating the next point in streamline
			MOUSEx += VELOCITYx*(double)cell_size/2;
			MOUSEy += VELOCITYy*(double)cell_size/2;


			/*if (count) {
				printf("----------------------------------------------\n");
				printf("Current point: %g %g\n", CURRENTx, CURRENTy);
				printf("Current grid: %d %d\n", GRIDx, GRIDy);
				printf("Calculated velocity: %g %g\n", VELOCITYx, VELOCITYy);
				printf("New point: %g %g\n", MOUSEx, MOUSEy);
				printf("%d\n", cell_size);
			
			}*/

			//Updating the X and Y of the next cell and assigning it to GRIDx and GRIDy
			GRIDx = (int)model_fft->clamp((double)(50 + 1) * ((double)MOUSEx / (double)winWidth));
			GRIDy = (int)model_fft->clamp((double)(50 + 1) * ((double)(winHeight - MOUSEy) / (double)winHeight));

			/*if (count) {
				printf("New grid: %d %d\n", GRIDx, GRIDy);
				
				count = 0;

			}*/
			
			//Draw Circle at the new center
			//drawCircle(MOUSEx, MOUSEy, 5);
			//draw line between the previous point and the new point
			drawLine(CURRENTx, CURRENTy, MOUSEx, MOUSEy);
			i++;
		}
		//make sure the streamline is drawn from the initial seed point again
		MOUSEx = start_point_X;
		MOUSEy = start_point_Y;
		GRIDx = start_grid_X;
		GRIDy = start_grid_Y;

	}
	
	
}


