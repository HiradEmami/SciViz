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

int main_window, control_window;


//do_one_simulation_step: Do one complete cycle of the simulation:
//      - set_forces:       
//      - solve:            read forces from the user
//      - diffuse_matter:   compute a new set of velocities
//      - gluPostRedisplay: draw a new visualization frame
void do_one_simulation_step(void)
{
	if (!frozen)
	{
		glutSetWindow(main_window);
		model_fft.set_forces(DIM);
		model_fft.solve(DIM);
		model_fft.diffuse_matter(DIM);
		glutPostRedisplay();
	}
}

void visualize(void)
{
	int        i, j, idx;
	fftw_real  wn = (fftw_real)view.winWidth / (fftw_real)(DIM + 1);   // Grid cell width
	fftw_real  hn = (fftw_real)view.winHeight / (fftw_real)(DIM + 1);  // Grid cell height
	float magnitude0, magnitude1, magnitude2, magnitude3;;

	if (view.draw_smoke)
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
					view.set_colormap(&color,model_fft.rho[idx0]);    glVertex2f(px0, py0);
					view.set_colormap(&color, model_fft.rho[idx1]);    glVertex2f(px1, py1);
					view.set_colormap(&color, model_fft.rho[idx2]);    glVertex2f(px2, py2);


					view.set_colormap(&color, model_fft.rho[idx0]);    glVertex2f(px0, py0);
					view.set_colormap(&color, model_fft.rho[idx2]);    glVertex2f(px2, py2);
					view.set_colormap(&color, model_fft.rho[idx3]);    glVertex2f(px3, py3);
				}
				
				else if (dataset == VELOCITY) {
					magnitude0 = sqrt((model_fft.vx[idx0] * model_fft.vx[idx0]) + (model_fft.vx[idx0] * model_fft.vx[idx0]));
					magnitude1 = sqrt((model_fft.vx[idx1] * model_fft.vx[idx1]) + (model_fft.vx[idx1] * model_fft.vx[idx1]));
					magnitude2 = sqrt((model_fft.vx[idx2] * model_fft.vx[idx2]) + (model_fft.vx[idx2] * model_fft.vx[idx2]));
					magnitude3 = sqrt((model_fft.vx[idx3] * model_fft.vx[idx3]) + (model_fft.vx[idx3] * model_fft.vx[idx3]));


					view.set_colormap(&color, magnitude0);    glVertex2f(px0, py0);
					view.set_colormap(&color, magnitude1);    glVertex2f(px1, py1);
					view.set_colormap(&color, magnitude2);    glVertex2f(px2, py2);

					view.set_colormap(&color, magnitude0);    glVertex2f(px0, py0);
					view.set_colormap(&color, magnitude2);    glVertex2f(px2, py2);
					view.set_colormap(&color, magnitude3);    glVertex2f(px3, py3);
				}

			}
		}
		glEnd();
	}

	if (view.draw_vecs)
	{
		glBegin(GL_LINES);				//draw velocities
		for (i = 0; i < DIM; i++)
			for (j = 0; j < DIM; j++)
			{
				idx = (j * DIM) + i;
				view.direction_to_color(model_fft.vx[idx], model_fft.vy[idx], view.color_dir);
				glVertex2f(wn + (fftw_real)i * wn, hn + (fftw_real)j * hn);
				glVertex2f((wn + (fftw_real)i * wn) + view.vec_scale * model_fft.vx[idx], (hn + (fftw_real)j * hn) + view.vec_scale * model_fft.vy[idx]);
			}
		glEnd();
	}


	//draw color bar
	view.draw_colorbar(&color);
	view.draw_numbers(&color);


}

void display()
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
	view.winWidth = w; view.winHeight = h;
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
	keyboard.keyboard(&view.scalar_col, &view.draw_vecs, &view.draw_smoke, &view.vec_scale, &view.color_dir,key,&color,&model_fft,  &frozen,&dataset,&VELOCITY);
	
}

int main(int argc, char **argv)
{
	// Initialize the main visualization window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(800, 600);

	main_window = glutCreateWindow("Real-time smoke simulation and visualization");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(do_one_simulation_step);
	glutKeyboardFunc(keyboardFunction);
	glutMotionFunc(drag);

	// Initialize models, view and controller
	model_fft = Model_fftw();
	color = Model_color();
	view = View_visualization();
	keyboard = Controller_keyboard();
	
	// define the control window and all of its functions
	GLUI* control_window = GLUI_Master.create_glui_subwindow(main_window, GLUI_SUBWINDOW_RIGHT);
	// add a panel for changing the dataset being colored
	GLUI_Panel* dataset_panel = control_window->add_panel("Dataset");
	GLUI_RadioGroup* dataset_buttons = control_window->add_radiogroup_to_panel(dataset_panel, &dataset);
	control_window->add_radiobutton_to_group(dataset_buttons, "Density");
	control_window->add_radiobutton_to_group(dataset_buttons, "Velocity");
	// add a panel for changing the colormap
	GLUI_Panel* colormap_panel = control_window->add_panel("Colormap");
	GLUI_RadioGroup* colormap_buttons = control_window->add_radiogroup_to_panel(colormap_panel, &view.scalar_col);	control_window->add_radiobutton_to_group(colormap_buttons, "Black-White");
	control_window->add_radiobutton_to_group(colormap_buttons, "Grayscale");
	control_window->add_radiobutton_to_group(colormap_buttons, "Rainbow");
	control_window->add_radiobutton_to_group(colormap_buttons, "Heatmap");
	control_window->add_radiobutton_to_group(colormap_buttons, "Diverging");
	control_window->add_radiobutton_to_group(colormap_buttons, "Blue-Yellow");
	


	


	
	model_fft.init_simulation(DIM);	//initialize the simulation data structures	
	glutMainLoop();			//calls do_one_simulation_step, keyboard, display, drag, reshape
	return 0;
}

