// Usage: Drag with the mouse to add smoke to the fluid. This will also move a "rotor" that disturbs 
//        the velocity field at the mouse location. Press the indicated keys to change options
//-------------------------------------------------------------------------------------------------- 

#include <rfftw.h>              //the numerical simulation Rlibrary
#include <GL/glut.h>            //the GLUT graphics library       
#include <iostream>				//for printing the help text
#include "Model_color.h"
#include "Model_fftw.h"
#include "Controller_keyboard.h"
#include "View_visualization.h"
#include <GL/glui.h>


const int DIM = 50;				//size of simulation grid


int DENSITY = 0;
int VELOCITY = 1;
int dataset = DENSITY;



int frozen = 0;					   //toggles on/off the animation




Model_fftw model_fft;
Model_color color;
View_visualization view;
Controller_keyboard keyboard;





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





void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	view.visualize(&color, &model_fft, &dataset, &VELOCITY, &DENSITY);
	glFlush();
	glutSwapBuffers();
}


// drag: When the user drags with the mouse, add a force that corresponds to the direction of the mouse
//       cursor movement. Also inject some new matter into the field at the mouse location.
void drag(int mx, int my)
{
	int xi, yi, X, Y; double  dx, dy, len;
	static int lmx = 0, lmy = 0;				//remembers last mouse location

												// Compute the array index that corresponds to the cursor location 
	xi = (int)model_fft.clamp((double)(DIM + 1) * ((double)mx / (double)view.winWidth));
	yi = (int)model_fft.clamp((double)(DIM + 1) * ((double)(view.winHeight - my) / (double)view.winHeight));

	X = xi; Y = yi;

	if (X > (DIM - 1))  X = DIM - 1; if (Y > (DIM - 1))  Y = DIM - 1;
	if (X < 0) X = 0; if (Y < 0) Y = 0;

	// Add force at the cursor location 
	my = view.winHeight - my;
	dx = mx - lmx; dy = my - lmy;
	len = sqrt(dx * dx + dy * dy);
	if (len != 0.0) { dx *= 0.1 / len; dy *= 0.1 / len; }
	model_fft.fx[Y * DIM + X] += dx;
	model_fft.fy[Y * DIM + X] += dy;
	model_fft.rho[Y * DIM + X] = 10.0f;
	lmx = mx; lmy = my;
}

void keyboardFunction(unsigned char key, int x, int y) {
	keyboard.keyboard( &view.scalar_col,&view.draw_vecs, &view.draw_smoke, &view.vec_scale, &view.color_dir,key,&color,&model_fft,  &frozen,&dataset,&VELOCITY);
	
}
void displayFunction() {
	view.display(&color, &model_fft, &dataset, &VELOCITY,&DENSITY);
}
void reshapeFunction(int w, int h) {
	view.reshape(w, h);
}
//main: The main program
using namespace std;
int main(int argc, char **argv)
{
	model_fft = Model_fftw();
	color = Model_color();
	view = View_visualization();
	keyboard = Controller_keyboard();
	

	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutCreateWindow("Real-time smoke simulation and visualization");
	glutDisplayFunc(display);
	glutReshapeFunc(reshapeFunction);
	glutIdleFunc(do_one_simulation_step);
	glutKeyboardFunc(keyboardFunction);
	glutMotionFunc(drag);
	
	

	model_fft.init_simulation(DIM);	//initialize the simulation data structures	
	glutMainLoop();			//calls do_one_simulation_step, keyboard, display, drag, reshape
	return 0;
}