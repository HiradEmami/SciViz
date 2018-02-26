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

// parameters for changing color dataset
int DENSITY = 0;
int VELOCITY = 1;
int FORCE = 2;
int dataset = DENSITY;

// parameters for changing glyph dataset
int SCALAR_DENSITY = 0;
int SCALAR_VELOCITY = 1;
int SCALAR_FORCE = 2;
int dataset_scalar = SCALAR_DENSITY;

int VECTOR_VELOCITY = 0;
int VECTOR_FORCE = 1;
int dataset_vector = VECTOR_VELOCITY;

int frozen = 0;					   //toggles on/off the animation

Model_fftw model_fft;
Model_color color;
View_visualization view;
Controller_keyboard keyboard;

int main_window = 1;
GLUI* control_window;

//do_one_simulation_step: Do one complete cycle of the simulation:
//      - set_forces:       
//      - solve:            read forces from the user
//      - diffuse_matter:   compute a new set of velocities
//      - gluPostRedisplay: draw a new visualization frame
void do_one_simulation_step(void)
{
	// sync control options
	control_window->sync_live();
	glutSetWindow(main_window);
	if (!frozen)
	{
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
	float value0, value1, value2, value3;

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

				// draw smoke density
				if (dataset == DENSITY) {
					// scalar values are simply the rho/density values
					value0 = model_fft.rho[idx0];
					value1 = model_fft.rho[idx1];
					value2 = model_fft.rho[idx2];
					value3 = model_fft.rho[idx3];
					
				}
				// draw smoke velocity
				else if (dataset == VELOCITY) {
					// scalar values are the magnitudes of the vectors
					value0 = sqrt((model_fft.vx[idx0] * model_fft.vx[idx0]) + (model_fft.vy[idx0] * model_fft.vy[idx0]));
					value1 = sqrt((model_fft.vx[idx1] * model_fft.vx[idx1]) + (model_fft.vy[idx1] * model_fft.vy[idx1]));
					value2 = sqrt((model_fft.vx[idx2] * model_fft.vx[idx2]) + (model_fft.vy[idx2] * model_fft.vy[idx2]));
					value3 = sqrt((model_fft.vx[idx3] * model_fft.vx[idx3]) + (model_fft.vy[idx3] * model_fft.vy[idx3]));

					// increase velocity scalar to make it more visible
					value0 *= 20;
					value1 *= 20;
					value2 *= 20;
					value3 *= 20;
					
				}

				// draw smoke force field
				else if (dataset == FORCE) {
					value0 = sqrt((model_fft.fx[idx0] * model_fft.fx[idx0]) + (model_fft.fy[idx0] * model_fft.fy[idx0]));
					value1 = sqrt((model_fft.fx[idx1] * model_fft.fx[idx1]) + (model_fft.fy[idx1] * model_fft.fy[idx1]));
					value2 = sqrt((model_fft.fx[idx2] * model_fft.fx[idx2]) + (model_fft.fy[idx2] * model_fft.fy[idx2]));
					value3 = sqrt((model_fft.fx[idx3] * model_fft.fx[idx3]) + (model_fft.fy[idx3] * model_fft.fy[idx3]));

					// increase force scalar to make it more visible
					value0 *= 20;
					value1 *= 20;
					value2 *= 20;
					value3 *= 20;
				

				}

				view.set_colormap(&color, value0, dataset);    glVertex2f(px0, py0);
				view.set_colormap(&color, value1, dataset);    glVertex2f(px1, py1);
				view.set_colormap(&color, value2, dataset);    glVertex2f(px2, py2);

				view.set_colormap(&color, value0, dataset);    glVertex2f(px0, py0);
				view.set_colormap(&color, value2, dataset);    glVertex2f(px2, py2);
				view.set_colormap(&color, value3, dataset);    glVertex2f(px3, py3);
			}
		}
		glEnd();
	}
	
	//draw vector field by using glyphs
	float x, y, scalar, magnitude;
	if (view.draw_vecs)
	{
		glBegin(GL_LINES);				
		for (i = 0; i < DIM; i++)
			for (j = 0; j < DIM; j++)
			{
				idx = (j * DIM) + i;
				// choose scalar for coloring 
				if (dataset_scalar == SCALAR_DENSITY) {
					scalar = model_fft.rho[idx];
				}
				else if (dataset_scalar == SCALAR_VELOCITY) {
					scalar = (model_fft.vx[idx] * model_fft.vx[idx]) + (model_fft.vy[idx] * model_fft.vy[idx]);
					scalar *= 100;
				}
				else if (dataset_scalar == SCALAR_FORCE) {
					scalar = (model_fft.fx[idx] * model_fft.fx[idx]) + (model_fft.fy[idx] * model_fft.fy[idx]);
					scalar *= 100;
				}
				//X and Y values depend on chosen dataset vector
				if (dataset_vector == VECTOR_VELOCITY) {
					x = model_fft.vx[idx];
					y = model_fft.vy[idx];
				}
				else if (dataset_vector == VECTOR_FORCE) {
					x = model_fft.fx[idx];
					y = model_fft.fy[idx];
				}
				// compute magnitude of chosen vector dataset
				magnitude = sqrt((x * x) + (y * y));
				magnitude *= 10;
				view.direction_to_color(x, y, scalar, view.scalar_col, color);
				// use x and y to draw corresponding direction of vector
				glVertex2f(wn + (fftw_real)i * wn, hn + (fftw_real)j * hn);
				glVertex2f((wn + (fftw_real)i * wn) + (view.vec_scale + (magnitude * view.vec_scale)) * x, (hn + (fftw_real)j * hn) + (view.vec_scale + (magnitude * view.vec_scale)) * y);
				//glVertex2f((wn + (fftw_real)i * wn) + view.vec_scale  * x, (hn + (fftw_real)j * hn) + view.vec_scale * y);
			}	
		glEnd();
	}


	//draw color bar
	view.draw_colorbar(&color);
	
	


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
	keyboard.keyboard(&view,key,&color,&model_fft,  &frozen,&dataset,&VELOCITY);
	
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
	//glutIdleFunc(do_one_simulation_step);
	GLUI_Master.set_glutIdleFunc(do_one_simulation_step);
	glutKeyboardFunc(keyboardFunction);
	glutMotionFunc(drag);

	// Initialize models, view and controller
	model_fft = Model_fftw();
	color = Model_color();
	view = View_visualization();
	keyboard = Controller_keyboard();

	model_fft.init_simulation(DIM);	//initialize the simulation data structures	
	
	
	// define the control window and all of its functions
	control_window = GLUI_Master.create_glui_subwindow(main_window, GLUI_SUBWINDOW_RIGHT);



	//--------------define all options for the COLORMAPPING assignment---------------------------//
	GLUI_Rollout* color_rollout = control_window->add_rollout("Colors", true);

		control_window->add_checkbox_to_panel(color_rollout, "Draw smoke", &view.draw_smoke);

		// add a panel for changing the dataset being colored
		GLUI_Panel* dataset_panel = control_window->add_panel_to_panel(color_rollout, "Dataset");
		GLUI_RadioGroup* dataset_buttons = control_window->add_radiogroup_to_panel(dataset_panel, &dataset);
		control_window->add_radiobutton_to_group(dataset_buttons, "Density");
		control_window->add_radiobutton_to_group(dataset_buttons, "||Velocity||");
		control_window->add_radiobutton_to_group(dataset_buttons, "||Force||");

		// add a panel for changing the colormap
		GLUI_Panel* colormap_panel = control_window->add_panel_to_panel(color_rollout, "Colormap");
		GLUI_RadioGroup* colormap_buttons = control_window->add_radiogroup_to_panel(colormap_panel, &view.scalar_col);
		control_window->add_radiobutton_to_group(colormap_buttons, "Black-White");
		control_window->add_radiobutton_to_group(colormap_buttons, "Grayscale");
		control_window->add_radiobutton_to_group(colormap_buttons, "Rainbow");
		control_window->add_radiobutton_to_group(colormap_buttons, "Heatmap");
		control_window->add_radiobutton_to_group(colormap_buttons, "Diverging");
		control_window->add_radiobutton_to_group(colormap_buttons, "Blue-Yellow");

		GLUI_Rollout* color_options_rollout = control_window->add_rollout_to_panel(color_rollout, "Color options", false);
		GLUI_Spinner* N_color_spinner = control_window->add_spinner_to_panel(color_options_rollout, "Colors", GLUI_LIVE_FLOAT, &color.NCOLORS);
		N_color_spinner->set_float_limits(2,255);
		//GLUI_Spinner* hue_spinner = control_window->add_spinner_to_panel(color_options_rollout, "Hue", GLUI_LIVE_FLOAT, &color.hue_change);
		//hue_spinner->set_float_limits(0, 1);
		//hue_spinner->set_float_val(1);
		//GLUI_Spinner* sat_spinner = control_window->add_spinner_to_panel(color_options_rollout, "Saturation", GLUI_LIVE_FLOAT, &color.saturation_change);
		//sat_spinner->set_float_limits(0, 1);
		//sat_spinner->set_float_val(1);

	//--------------define all options for the GLYPH assignment---------------------------//
		GLUI_Rollout* glyph_rollout = control_window->add_rollout("Glyphs", true);
			control_window->add_checkbox_to_panel(glyph_rollout, "Draw glyphs", &view.draw_vecs);
			control_window->add_checkbox_to_panel(glyph_rollout, "Color glyphs", &view.color_dir);
			GLUI_Panel* scalar_panel = control_window->add_panel_to_panel(glyph_rollout, "Scalar");
			GLUI_Panel* vector_panel = control_window->add_panel_to_panel(glyph_rollout, "Vector");
			GLUI_RadioGroup* scalar_buttons = control_window->add_radiogroup_to_panel(scalar_panel, &dataset_scalar);
			GLUI_RadioGroup* vector_buttons = control_window->add_radiogroup_to_panel(vector_panel, &dataset_vector);
			control_window->add_radiobutton_to_group(scalar_buttons, "Density");
			control_window->add_radiobutton_to_group(scalar_buttons, "Velocity");
			control_window->add_radiobutton_to_group(scalar_buttons, "Force");

			control_window->add_radiobutton_to_group(vector_buttons, "Velocity");
			control_window->add_radiobutton_to_group(vector_buttons, "Force");

			
			

	
	glutMainLoop();			//calls do_one_simulation_step, keyboard, display, drag, reshape
	return 0;
}

