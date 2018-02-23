// Usage: Drag with the mouse to add smoke to the fluid. This will also move a "rotor" that disturbs 
//        the velocity field at the mouse location. Press the indicated keys to change options
//-------------------------------------------------------------------------------------------------- 

#include <rfftw.h>              //the numerical simulation Rlibrary
#include <GL/glut.h>            //the GLUT graphics library       
#include <iostream>				//for printing the help text
#include "Color.h"
#include "Model_fftw.h"
#include <GL/glui.h>


const int DIM = 50;				//size of simulation grid

								//--- VISUALIZATION PARAMETERS ---------------------------------------------------------------------
int   winWidth, winHeight;      //size of the graphics window, in pixels
int   color_dir = 0;            //use direction color-coding or not
float vec_scale = 1000;			//scaling of hedgehogs
int   draw_smoke = 1;           //draw the smoke or not
int   draw_vecs = 0;            //draw the vector field or not
const int COLOR_BLACKWHITE = 0;   //different types of color mapping: black-and-white, grayscale, rainbow, banded
const int COLOR_GRAYSCALE = 1;
const int COLOR_RAINBOW = 2;
const int COLOR_HEATMAP = 3;
const int COLOR_DIVERGING = 4;
const int COLOR_TWOCOLORS = 5;

const int DENSITY = 0;
const int VELOCITY = 1;
int dataset = DENSITY;


int scalar_col = COLOR_HEATMAP;   //set initial colormap to black and white
									 //method for scalar coloring
int frozen = 0;					   //toggles on/off the animation
float colorbar_width = 50;
int colorbar_height;

// parameters for scaling and clamping
float scale_min = 0.0;
float scale_max = 1.0;
float scale_step = 0.01;
int NCOLORS = 255;
float saturation_change = 1;
float hue_change = 1;

Model_fftw model_fft;



//do_one_simulation_step: Do one complete cycle of the simulation:
//      - set_forces:       
//      - solve:            read forces from the user
//      - diffuse_matter:   compute a new set of velocities
//      - gluPostRedisplay: draw a new visualization frame
void do_one_simulation_step(void)
{
	if (!frozen)
	{
		model_fft.set_forces(DIM);
		model_fft.solve(DIM);
		model_fft.diffuse_matter(DIM);
		glutPostRedisplay();
	}
}


//------ VISUALIZATION CODE STARTS HERE -----------------------------------------------------------------
void rgb2hsv(float r, float g, float b, float *h, float *s, float *v)
{
	float M = fmax(r, fmax(g, b));
	float m = fmin(r, fmin(g, b));
	float d = M - m;
	*v = M; //value = max( r ,g ,b)
	*s = (M>0.00001) ? d / M : 0; //saturation
	if(s == 0) h = 0; //achromatic case , hue=0 by convention
	else //chromatic case
	{
	if(r == M) *h = (g-b) / d;
	else if(g == M) *h = 2 + (b-r) / d;
	else *h = 4 + (r-g) / d;
	*h /= 6;
	if(h<0) h += 1;
	}

	// user controlled h and s
	*s *= saturation_change;
	*h *= hue_change;
}

void hsv2rgb(float h, float s, float v, float *r, float *g, float *b) {
	int hueCase = (int)(h * 6);
	float frac = 6 * h - hueCase;
	float lx = v *(1 - s);
	float ly = v*(1 - s * frac);
	float lz = v*(1 - s *(1 - frac));
	switch(hueCase)
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

void interpolate(float value, float* R, float* G, float* B, float r1, float g1, float b1, float r2, float g2, float b2)
{
	*R = r1 * (1.0 - value) + value * r2;
	*G = g1 * (1.0 - value) + value * g2;
	*B = b1 * (1.0 - value) + value * b2;
}

//rainbow: Implements a color palette, mapping the scalar 'value' to a rainbow color RGB
void rainbow(float value, float* R, float* G, float* B)
{
	const float dx = 0.8;
	if (value<0) value = 0; if (value>1) value = 1;
	value = (6 - 2 * dx)*value + dx;
	*R = fmax(0.0, (3 - fabs(value - 4) - fabs(value - 5)));
	*G = fmax(0.0, (4 - fabs(value - 2) - fabs(value - 4)));
	*B = fmax(0.0, (3 - fabs(value - 1) - fabs(value - 2)));
}
void grayscale(float value, float* R, float* G, float* B)
{
	if (value<0) value = 0; if (value>1) value = 1;
	value = value / 3;
	*R = *G = *B = value;
}

void heatmap(float value, float* R, float* G, float* B)
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

void blackwhite(float value, float* R, float* G, float* B)
{
	*R = *G = *B = value;
}




void diverging(float value, float* R, float* G, float* B)
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
void set_colormap(float vy)
{	
	vy *= NCOLORS; vy = (int)vy; vy /= NCOLORS;
	float R, G, B;
	if (scalar_col == COLOR_BLACKWHITE)
		blackwhite(vy, &R, &G, &B);
	else if (scalar_col == COLOR_GRAYSCALE)
		grayscale(vy, &R, &G, &B);
	else if (scalar_col == COLOR_RAINBOW)
		rainbow(vy, &R, &G, &B);
	else if (scalar_col == COLOR_HEATMAP) {
		heatmap(vy, &R, &G, &B);
	}
	else if (scalar_col == COLOR_DIVERGING) {
		diverging(vy, &R, &G, &B);
	}
	else if (scalar_col == COLOR_TWOCOLORS) {
		interpolate(vy, &R, &G, &B,0,0,1,1,1,0);
	}

	float h, s, v;
	rgb2hsv(R, G, B, &h, &s, &v);
	hsv2rgb(h, s, v, &R, &G, &B);

	glColor3f(R, G, B);
}


//direction_to_color: Set the current color by mapping a direction vector (x,y), using
//                    the color mapping method 'method'. If method==1, map the vector direction
//                    using a rainbow colormap. If method==0, simply use the white color
void direction_to_color(float x, float y, int method)
{
	float r, g, b, f;
	if (method)
	{
		f = atan2(y, x) / 3.1415927 + 1;
		r = f;
		if (r > 1) r = 2 - r;
		g = f + .66667;
		if (g > 2) g -= 2;
		if (g > 1) g = 2 - g;
		b = f + 2 * .66667;
		if (b > 2) b -= 2;
		if (b > 1) b = 2 - b;
	}
	else
	{
		r = g = b = 1;
	}
	glColor3f(r, g, b);
}

//compute the rgb values given the current segment and the current colormap
void compute_RGB(float value, float* R, float* G, float* B) {
	switch (scalar_col) {
	case COLOR_BLACKWHITE:
		blackwhite(value, R, G, B);
		break;
	case COLOR_GRAYSCALE:
		grayscale(value, R, G, B);
		break;
	case COLOR_RAINBOW:
		rainbow(value, R, G, B);
		break;
	case COLOR_HEATMAP:
		heatmap(value, R, G, B);
		break;
	case COLOR_DIVERGING:
		diverging(value, R, G, B);
		break;
	case COLOR_TWOCOLORS:
		interpolate(value, R, G, B, 0, 0, 1, 1, 1, 0);
		break;
	}
	

}


//draw a colorbar with the currently selected colormap
void draw_colorbar() {

	//the amount of 'strips'
	int segments = 50;
	float R, G, B, value;
	//each 'strip' has the same height and width
	float segment_height = winHeight / segments;

	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i < segments + 1; i++) {
		//the value is in the range [0,1], computed by block number / N
		value = (float)i / (float)segments;
		//compute the RGB values using the value of the current strip and the current colormap
		compute_RGB(value, &R, &G, &B);
		float h, s, v;
		rgb2hsv(R, G, B, &h, &s, &v);
		hsv2rgb(h, s, v, &R, &G, &B);
		glColor3f(R, G, B);
		//draw the strips with two vertices
		glVertex2f(winWidth - colorbar_width, i*segment_height);
		glVertex2f(winWidth, i*segment_height);
	}

	glEnd();

}

void draw_numbers() {
	glColor3f(1, 1, 1);
	glRasterPos2f(winWidth-20, winHeight-20);
	
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, (float)scale_max);
}


//visualize: This is the main visualization function
void visualize(void)
{
	int        i, j, idx;
	fftw_real  wn = (fftw_real)winWidth / (fftw_real)(DIM + 1);   // Grid cell width
	fftw_real  hn = (fftw_real)winHeight / (fftw_real)(DIM + 1);  // Grid cell height
	float magnitude;

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

				if (dataset == DENSITY) {

					set_colormap(model_fft.rho[idx0]);    glVertex2f(px0, py0);
					set_colormap(model_fft.rho[idx1]);    glVertex2f(px1, py1);
					set_colormap(model_fft.rho[idx2]);    glVertex2f(px2, py2);


					set_colormap(model_fft.rho[idx0]);    glVertex2f(px0, py0);
					set_colormap(model_fft.rho[idx2]);    glVertex2f(px2, py2);
					set_colormap(model_fft.rho[idx3]);    glVertex2f(px3, py3);
				}
				else if (dataset == VELOCITY) {
					magnitude = sqrt((model_fft.vx[idx0] * model_fft.vx[idx0]) + (model_fft.vx[idx0] * model_fft.vx[idx0]));
					set_colormap(magnitude);
					glVertex2f(wn + (fftw_real)i * wn, hn + (fftw_real)j * hn);
					glVertex2f((wn + (fftw_real)i * wn) + vec_scale * model_fft.vx[idx0], (hn + (fftw_real)j * hn) + vec_scale * model_fft.vy[idx0]);

					set_colormap(model_fft.rho[idx0]);    glVertex2f(px0, py0);
					set_colormap(model_fft.rho[idx1]);    glVertex2f(px1, py1);
					set_colormap(model_fft.rho[idx2]);    glVertex2f(px2, py2);


					set_colormap(model_fft.rho[idx0]);    glVertex2f(px0, py0);
					set_colormap(model_fft.rho[idx2]);    glVertex2f(px2, py2);
					set_colormap(model_fft.rho[idx3]);    glVertex2f(px3, py3);
				}
				else if (dataset == VELOCITY) {
					magnitude = sqrt((model_fft.vx[idx0] * model_fft.vx[idx0]) + (model_fft.vx[idx0] * model_fft.vx[idx0]));
					set_colormap(magnitude);
					glVertex2f(wn + (fftw_real)i * wn, hn + (fftw_real)j * hn);
					glVertex2f((wn + (fftw_real)i * wn) + vec_scale * model_fft.vx[idx0], (hn + (fftw_real)j * hn) + vec_scale * model_fft.vy[idx0]);

				}

			}
		}
		glEnd();
	}

	if (draw_vecs)
	{
		glBegin(GL_LINES);				//draw velocities
		for (i = 0; i < DIM; i++)
			for (j = 0; j < DIM; j++)
			{
				idx = (j * DIM) + i;
				direction_to_color(model_fft.vx[idx], model_fft.vy[idx], color_dir);
				glVertex2f(wn + (fftw_real)i * wn, hn + (fftw_real)j * hn);
				glVertex2f((wn + (fftw_real)i * wn) + vec_scale * model_fft.vx[idx], (hn + (fftw_real)j * hn) + vec_scale * model_fft.vy[idx]);
			}
		glEnd();
	}
	/*if (dataset == VELOCITY) {
		float magnitude;
		glBegin(GL_LINES);				//draw velocity magnitude
		for (i = 0; i < DIM; i++)
			for (j = 0; j < DIM; j++)
			{
				idx = (j * DIM) + i;
				magnitude = sqrt((vx[idx] * vx[idx]) + (vx[idx] * vx[idx]));
				set_colormap(magnitude);
				glVertex2f(wn + (fftw_real)i * wn, hn + (fftw_real)j * hn);
				glVertex2f((wn + (fftw_real)i * wn) + vec_scale * vx[idx], (hn + (fftw_real)j * hn) + vec_scale * vy[idx]);
			}
		glEnd();
	}*/
	
	//draw color bar
	draw_colorbar();
	draw_numbers();
	

}


//------ INTERACTION CODE STARTS HERE -----------------------------------------------------------------

//display: Handle window redrawing events. Simply delegates to visualize().
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	visualize();
	glFlush();
	glutSwapBuffers();
}

//reshape: Handle window resizing (reshaping) events
void reshape(int w, int h)
{
	glViewport(0.0f, 0.0f, (GLfloat)w, (GLfloat)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, (GLdouble)w, 0.0, (GLdouble)h);
	winWidth = w; winHeight = h;
}

//keyboard: Handle key presses
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 't': model_fft.dt -= 0.001; break;
	case 'T': model_fft.dt += 0.001; break;
	case 'c': color_dir = 1 - color_dir; break;
	case 'S': vec_scale *= 1.2; break;
	case 's': vec_scale *= 0.8; break;
	case 'V': model_fft.visc *= 5; break;
	case 'vy': model_fft.visc *= 0.2; break;
	case 'x': draw_smoke = 1 - draw_smoke;
		if (draw_smoke == 0) draw_vecs = 1; break;
	case 'y': draw_vecs = 1 - draw_vecs;
		if (draw_vecs == 0) draw_smoke = 1; break;
	case 'm': scalar_col++; if (scalar_col>COLOR_TWOCOLORS) scalar_col = COLOR_BLACKWHITE; break;
	case 'a': frozen = 1 - frozen; break;
	case 'q': exit(0);
	case '4': scale_min = scale_min - scale_step; break;
	case '6': scale_min = scale_min + scale_step; break;
	case '2': scale_max = scale_max - scale_step; break;
	case '8': scale_max = scale_max + scale_step; break;
	case '9': scale_min = 0; scale_max = 1; break;
	case 'n': NCOLORS -= 1; if (NCOLORS < 2) NCOLORS = 2;  break;
	case 'N': NCOLORS += 1;  if (NCOLORS > 256) NCOLORS = 256; break;
	case 'r': saturation_change -= 0.01; if (saturation_change < 0) saturation_change = 0;  break;
	case 'R': saturation_change += 0.01; if (saturation_change > 1) saturation_change = 1;  break;
	case 'h': hue_change -= 0.01; if (hue_change < 0) hue_change = 0;  break;
	case 'H': hue_change += 0.01; if (hue_change > 1) hue_change = 1;  break;
	case 'd': dataset += 1; if (dataset > VELOCITY) dataset = 0; break;
	}
}

// drag: When the user drags with the mouse, add a force that corresponds to the direction of the mouse
//       cursor movement. Also inject some new matter into the field at the mouse location.
void drag(int mx, int my)
{
	int xi, yi, X, Y; double  dx, dy, len;
	static int lmx = 0, lmy = 0;				//remembers last mouse location

												// Compute the array index that corresponds to the cursor location 
	xi = (int)model_fft.clamp((double)(DIM + 1) * ((double)mx / (double)winWidth));
	yi = (int)model_fft.clamp((double)(DIM + 1) * ((double)(winHeight - my) / (double)winHeight));

	X = xi; Y = yi;

	if (X > (DIM - 1))  X = DIM - 1; if (Y > (DIM - 1))  Y = DIM - 1;
	if (X < 0) X = 0; if (Y < 0) Y = 0;

	// Add force at the cursor location 
	my = winHeight - my;
	dx = mx - lmx; dy = my - lmy;
	len = sqrt(dx * dx + dy * dy);
	if (len != 0.0) { dx *= 0.1 / len; dy *= 0.1 / len; }
	model_fft.fx[Y * DIM + X] += dx;
	model_fft.fy[Y * DIM + X] += dy;
	model_fft.rho[Y * DIM + X] = 10.0f;
	lmx = mx; lmy = my;
}


//main: The main program
using namespace std;
int main(int argc, char **argv)
{
	model_fft = Model_fftw();
	cout << "Fluid Flow Simulation and Visualization\n";
	cout << "=======================================\n";
	cout << "Click and drag the mouse to steer the flow!\n";
	cout << "T/t:   increase/decrease simulation timestep\n";
	cout << "S/s:   increase/decrease hedgehog scaling\n";
	cout << "c:     toggle direction coloring on/off\n";
	cout << "V/vy:   increase decrease fluid viscosity\n";
	cout << "x:     toggle drawing matter on/off\n";
	cout << "y:     toggle drawing hedgehogs on/off\n";
	cout << "m:     toggle thru scalar coloring\n";
	cout << "a:     toggle the animation on/off\n";
	cout << "4:		lower fmin\n";
	cout << "6:		increase fmin\n";
	cout << "2:		lower fmax\n";
	cout << "8:		increase fmax\n";
	cout << "9:		reset fmin and fmax\n";
	cout << "n:		decrease number of colors used\n";
	cout << "N:		increase number of colors used\n";
	cout << "q:     quit\n\n";

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutCreateWindow("Real-time smoke simulation and visualization");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(do_one_simulation_step);
	glutKeyboardFunc(keyboard);
	glutMotionFunc(drag);
	GLUI *glut = GLUI_Master.create_glui("test");
	model_fft.init_simulation(DIM);	//initialize the simulation data structures	
	glutMainLoop();			//calls do_one_simulation_step, keyboard, display, drag, reshape
	return 0;
}