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

View_visualization::View_visualization()
{
	 DIM = 3;
	 color_dir = 0;            //use direction color-coding or not
	 vec_scale = 1000;			//scaling of hedgehogs
	 draw_smoke = 0;           //draw the smoke or not
	 draw_vecs = 1;            //draw the vector field or not
	 use_clamp = 1;
	 colorbar_width = 50;
	 COLOR_BLACKWHITE = 0;
	 COLOR_GRAYSCALE = 1;
	 COLOR_RAINBOW = 2;
	 COLOR_HEATMAP = 3;
	 COLOR_DIVERGING = 4;
	 COLOR_TWOCOLORS = 5;
	 scalar_col = 0;
	 //glyph parameters
	 glyph_type = 0;
	 //draw normal vector glyphs or gradient glyphs
	 vector_type = 0;
	 STANDARD = 0;
	 GRADIENT = 1;

	 glyph_samplingrateX = 1;
	 glyph_samplingrateY = 1;

	 //streamline parameters
	 draw_steamline = 0; 
	 MOUSEx = 0;
	 MOUSEy = 0;


	 


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
void View_visualization::direction_to_color(float scalar, int colormap, Model_color color)
{
	scalar = color.clamp(scalar);
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
void View_visualization::draw_colorbar(Model_color* color) {
    // we use NCOLORS amount of rectangles in the colorbar
	float R, G, B, H, S, V;
	//each piece has the same height and width
	float colorbar_width = winWidth - (winWidth / 5);
	float segment_width = colorbar_width / (color->NCOLORS + 1);
	float segment_value = 0;
	float colorbar_height = winHeight / 10;

	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= color->NCOLORS; i++) {
		segment_value = (float)i / color->NCOLORS;
		compute_RGB(&*color, segment_value, &R, &G, &B);
		color->rgb2hsv(R, G, B, &H, &S, &V);
		color->hsv2rgb(H, S, V, &R, &G, &B);
		glColor3f(R, G, B);
		glVertex2f((i * segment_width), 0);
		glVertex2f((i * segment_width), colorbar_height);

		glVertex2f((i * segment_width) + segment_width, 0);
		glVertex2f((i * segment_width) + segment_width, colorbar_height);

	}
	glEnd();
	
	draw_number(&*color, std::to_string(color->min), 5);
	draw_number(&*color, std::to_string((color->max + color->min) / 2), winWidth / 2 - winWidth / 8);
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
void View_visualization::drawCircle(GLfloat cx, GLfloat cy, GLfloat radius) {
	float i = 0.0f;

	glBegin(GL_TRIANGLE_FAN);

	glVertex2f(cx, cy); // Center
	for (i = 0.0f; i <= 360; i++)
		glVertex2f(radius*cos(M_PI * i / 180.0) + cx, radius*sin(M_PI * i / 180.0) + cy);

	glEnd();

}


// apply the inverse coordinate transform to get the reference cell coordinates for the given point p
void View_visualization::get_reference_coordinates(int p, int p1, int p2, int p3, int p4, int* r, int* s) {
	*r = ((p - p1) * (p2 - p1)) / ((p2 - p1) * (p2 - p1));
	*s = ((p - p1) * (p4 - p1)) / ((p4 - p1) * (p4 - p1));
}

void View_visualization::visualize(int DIM, Model_fftw* model_fft,Model_color* color,int* DENSITY, int* VELOCITY, int* FORCE, int* dataset,
	int* SCALAR_DENSITY, int* SCALAR_VELOCITY, int* SCALAR_FORCE, int* dataset_scalar,
	int* VECTOR_VELOCITY, int* VECTOR_FORCE,int* dataset_vector)
{
	int        i, j, idx;
	fftw_real  wn = (fftw_real)winWidth / (fftw_real)(DIM + 1);   // Grid cell width
	fftw_real  hn = (fftw_real)winHeight / (fftw_real)(DIM + 1);  // Grid cell height
	float value0, value1, value2, value3;
	int idx0, idx1, idx2, idx3;

	if (draw_smoke)
	{
		\
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

				// Draw the smoke at all four vertices, by drawing two triangles

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
	
	/*glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);*/
	//gluLookAt(0, 0, 2, 0, 0, 0, 0, 1, 0);

	//draw vector field by using glyphs
	
	float x, y, scalar, magnitude;
	if (draw_vecs)
	{
		//glBegin(GL_LINES);
		//GL_TRIANGLES
		//set_Glyph_type();
		for (i = 0; i < DIM; i+= glyph_samplingrateX)
			for (j = 0; j < DIM; j+= glyph_samplingrateY)
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


				if (vector_type == STANDARD) {
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

					// use x and y to draw corresponding direction of vector
					//glVertex2f(wn + (fftw_real)i * wn, hn + (fftw_real)j * hn);
					//glVertex2f((wn + (fftw_real)i * wn) + (vec_scale + (magnitude * vec_scale)) * x, (hn + (fftw_real)j * hn) + (vec_scale + (magnitude * vec_scale)) * y);
					//glVertex2f((wn + (fftw_real)i * wn) + view.vec_scale  * x, (hn + (fftw_real)j * hn) + view.vec_scale * y);
				}

				// draw the gradient of either density or velocity magnitude
				else if (vector_type == GRADIENT) {
				
					idx0 = (j * DIM) + i;
					idx1 = ((j + 1) * DIM) + i;
					idx2 = ((j + 1) * DIM) + (i + 1);
					idx3 = (j * DIM) + (i + 1);
					
					// compute derivative of vertical sides of cell
					x = (0.5*(model_fft->rho[idx1] - model_fft->rho[idx0]) / wn) + (0.5*(model_fft->rho[idx2] - model_fft->rho[idx3]) / wn);
					y = (0.5*(model_fft->rho[idx3] - model_fft->rho[idx0]) / wn) + (0.5*(model_fft->rho[idx2] - model_fft->rho[idx1]) / hn);

					

				}

				// Scale size of glyph to the cell width
				float radius = wn/2;
				float angle;			
				// Draw glyph by using the magnitude of the vector and its orientation
				glPushMatrix();			
				// Translate glyph to current cell
				glTranslatef(wn + (fftw_real)i * wn, hn + (fftw_real)j * hn, 1);
				// Compute arctangent of vector y and x values
				angle = atan2(y, x);
				angle = 180 * angle / M_PI;
				// Rotate glyph according to computed vector orientation
				glRotatef(90-angle, 0.0, 0.0, -1.0);
				glRotatef(-90.0, 1.0, 0.0, 0.0);
				// Draw the solid cones with specified radius
				glutSolidCone(radius/2, radius, 3,3);			
				// Color the glpyhs
				direction_to_color(scalar, scalar_col, *color);
				glPopMatrix(); 					
			}
		glEnd();	
	}

	//glDisable(GL_LIGHTING);
	//draw color bar
	draw_colorbar(&*color);;
	/*if (draw_steamline == 1) {
		drawCircle(MOUSEx, MOUSEy, 5);
	}*/


}

void View_visualization::display_Steamline(Model_fftw* model_fft) {


}


