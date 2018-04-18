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
#include "controller_viewInteraction.h"
#include <GL/glui.h>
#include "wtypes.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <queue>
using std::queue;


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

//int horizontal, vertical;

Model_fftw model_fft;
Model_color color;
View_visualization view;
Controller_keyboard keyboard;
controller_viewInteraction interaction;

int main_window = 1;
GLUI* control_window;

//slice parameters
queue<Model_fftw> modelQueue;

int slice_size = 20;
int timer = 0;


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
	if (view.draw_slices) {
		if (modelQueue.size() <= slice_size - 1) {
			modelQueue.push(model_fft);
		}
		else {
			if (timer >= 0) {
				timer = 0;
				modelQueue.pop();
				modelQueue.push(model_fft);
			}
			else {
				timer++;
			}
			
		}
	}
}

void visualize(void)
{
	if (view.draw_slices ) {
		float z, zstep, alpha, alphastep, shift;
		z = -3;
		zstep = 0.5 / slice_size;
		alpha = 1.0;
		alphastep = 0.9 / slice_size;
		// create a temporary copy of the queue
		int size = modelQueue.size();
		queue<Model_fftw> queueCopy;


		for (int i = 0; i < size; i++) {
			// add model to copy
			queueCopy.push(modelQueue.front());
			modelQueue.pop();
		}
		
		// visualize all the slices

		for (int i = 0; i < size; i++) {
			z = z - 0.2;
			//z = z + ((i * zstep));
			//alpha = alpha - (i * alphastep);
			shift = (i * 150) + 700;
			//printf("alphastep: %g\n", alphastep);
			//printf("i: %d, alpha:%g\n", i, alpha - (i * alphastep));
			
			view.visualize(DIM, &queueCopy.front(), &color, &DENSITY, &VELOCITY, &FORCE, &dataset,
				&SCALAR_DENSITY, &SCALAR_VELOCITY, &SCALAR_FORCE, &dataset_scalar,
				z, alpha - (i * alphastep), shift);


			modelQueue.push(queueCopy.front());
			queueCopy.pop();
		}
	}
	else {
		view.visualize(DIM, &model_fft, &color, &DENSITY, &VELOCITY, &FORCE, &dataset,
			&SCALAR_DENSITY, &SCALAR_VELOCITY, &SCALAR_FORCE, &dataset_scalar,
			-1, 1.0, 0);
	}


}

//reshape: Handle window resizing (reshaping) events
void reshape(int w, int h)
{
	interaction.reshape(&view,&w,&h);
}
void keyboardFunction(unsigned char key, int x, int y) {
	keyboard.keyboard(&view,key,&color,&model_fft,  &frozen,&dataset,&VELOCITY);
}
void drag(int mx, int my) {
	interaction.drag(&view,&model_fft, DIM, &mx, &my);
}

void getPosition(int btn, int state, int x, int y) {
	// flip the y value
	y = view.winHeight - y;
	interaction.mouse(&btn, &state,&x, &y,&view,&model_fft,DIM);
}

// Get the horizontal and vertical screen sizes in pixel
void GetDesktopResolution(int* horizontal, int* vertical)
{

	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	*horizontal = desktop.right;
	*vertical = desktop.bottom;
}


void display()
{
	int horizontal, vertical;
	GetDesktopResolution(&horizontal, &vertical);
	
	//gluOrtho2D(0.0, (GLdouble)(horizontal), 0.0, (GLdouble)(vertical));
	/*
	glViewport(0.0f, 0.0f, (GLfloat)(horizontal), (GLfloat)(vertical));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(M_PI / 2, 1.0, 1, 1000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 0, 0, 0, -1, 0, 1, 0);
	
	visualize();
	glFlush();
	glutSwapBuffers();*/

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glViewport(0.0f, 0.0f, (GLfloat)(horizontal), (GLfloat)(vertical));
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	visualize();
	glFlush();
	glutSwapBuffers();

}

int main(int argc, char **argv)
{
	float winWidth, winHeight;
	winWidth = 1000;
	winHeight = 800;
	// Initialize models, view and controller
	model_fft = Model_fftw();
	color = Model_color();
	view = View_visualization(winWidth, winHeight);
	keyboard = Controller_keyboard();
	interaction = controller_viewInteraction();



	// Initialize the main visualization window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	//int horizontal, vertical;
	//GetDesktopResolution(&horizontal, &vertical);
	//glutInitWindowSize(horizontal, vertical);
	glutInitWindowSize(winWidth, winHeight);

	main_window = glutCreateWindow("Real-time smoke simulation and visualization");
	GLUI_Master.set_glutDisplayFunc(display);
	//glutDisplayFunc(display);
	//GLUI_Master.ReshapeFunc(reshape);
	GLUI_Master.set_glutReshapeFunc(reshape);
	//glutIdleFunc(do_one_simulation_step);
	GLUI_Master.set_glutIdleFunc(do_one_simulation_step);
	glutKeyboardFunc(keyboardFunction);
	glutMotionFunc(drag);
	glutMouseFunc(getPosition);

	

	model_fft.init_simulation(DIM);	//initialize the simulation data structures	
	
	
	// define the control window and all of its functions
	//control_window = GLUI_Master.create_glui("Control",GLUI_SUBWINDOW_RIGHT);
	//control_window = GLUI_Master.create_glui_subwindow(main_window, GLUI_SUBWINDOW_RIGHT);
	control_window = GLUI_Master.create_glui("Control panel");
	

	//--------------define all options for the COLORMAPPING assignment---------------------------//
	GLUI_Rollout* color_rollout = control_window->add_rollout("Colors", false);

		control_window->add_checkbox_to_panel(color_rollout, "Draw smoke", &view.draw_smoke);

		GLUI_RadioGroup* clamp_scale_buttons = control_window->add_radiogroup_to_panel(color_rollout, &view.use_clamp);

		control_window->add_radiobutton_to_group(clamp_scale_buttons, "Scale");
		control_window->add_radiobutton_to_group(clamp_scale_buttons, "Clamp");

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

		GLUI_Rollout* color_options_rollout = control_window->add_rollout_to_panel(color_rollout, "Color options", true);
		GLUI_Spinner* N_color_spinner = control_window->add_spinner_to_panel(color_options_rollout, "Colorbands", GLUI_SPINNER_INT, &color.NCOLORS);
		N_color_spinner->set_float_limits(1,255);
		/*GLUI_RadioGroup* scale_clamp = control_window->add_radiogroup_to_panel(color_rollout, &view.bool_clamp_scale);
		control_window->add_radiobutton_to_group(scale_clamp, "Use Scaling");
		control_window->add_radiobutton_to_group(scale_clamp, "Use Clamping");*/

		GLUI_Spinner* min_spinner = control_window->add_spinner_to_panel(color_options_rollout, "Fmin", GLUI_SPINNER_FLOAT, &color.density_min);
		GLUI_Spinner* max_spinner = control_window->add_spinner_to_panel(color_options_rollout, "Fmax", GLUI_SPINNER_FLOAT, &color.density_max);
		min_spinner->set_float_limits(0, 1);
		max_spinner->set_float_limits(0, 1);
		
		GLUI_Spinner* hue_spinner = control_window->add_spinner_to_panel(color_options_rollout, "Hue", GLUI_SPINNER_FLOAT, &color.hue_change);
		hue_spinner->set_float_limits(0, 1);
		hue_spinner->set_float_val(1);

		GLUI_Spinner* sat_spinner = control_window->add_spinner_to_panel(color_options_rollout, "Saturation", GLUI_SPINNER_FLOAT, &color.saturation_change);
		sat_spinner->set_float_limits(0, 1);
		sat_spinner->set_float_val(1);
		

	//--------------define all options for the GLYPH assignment---------------------------//
		GLUI_Rollout* glyph_rollout = control_window->add_rollout("Glyphs", true);
			control_window->add_checkbox_to_panel(glyph_rollout, "Draw glyphs", &view.draw_vecs);
			control_window->add_checkbox_to_panel(glyph_rollout, "Color glyphs", &view.color_dir);

			GLUI_Panel* field_panel = control_window->add_panel_to_panel(glyph_rollout, "Vector field");
			GLUI_Panel* type_panel = control_window->add_panel_to_panel(glyph_rollout, "Glyph type");
			GLUI_Panel* scalar_panel = control_window->add_panel_to_panel(glyph_rollout, "Scalar dataset");
			

			GLUI_RadioGroup* field_buttons = control_window->add_radiogroup_to_panel(field_panel, &view.vector_type);
			GLUI_RadioGroup* type_buttons = control_window->add_radiogroup_to_panel(type_panel, &view.glyph_type);
			GLUI_RadioGroup* scalar_buttons = control_window->add_radiogroup_to_panel(scalar_panel, &dataset_scalar);
		

			control_window->add_radiobutton_to_group(field_buttons, "Velocity");
			control_window->add_radiobutton_to_group(field_buttons, "Force");
			control_window->add_radiobutton_to_group(field_buttons, "Density Gradient");
			control_window->add_radiobutton_to_group(field_buttons, "Velocity Gradient");

			control_window->add_radiobutton_to_group(type_buttons, "Lines");
			control_window->add_radiobutton_to_group(type_buttons, "Cones");
			control_window->add_radiobutton_to_group(type_buttons, "Arrows");

			control_window->add_radiobutton_to_group(scalar_buttons, "Density");
			control_window->add_radiobutton_to_group(scalar_buttons, "Velocity");
			control_window->add_radiobutton_to_group(scalar_buttons, "Force");

		


			GLUI_Spinner* N_sampleX_spinner = control_window->add_spinner_to_panel(glyph_rollout, "Row samples", GLUI_SPINNER_INT, &view.glyph_samplesY);
			N_sampleX_spinner->set_float_limits(1, 50);

			GLUI_Spinner* N_sampleY_spinner = control_window->add_spinner_to_panel(glyph_rollout, "Column samples", GLUI_SPINNER_INT, &view.glyph_samplesX);
			N_sampleY_spinner->set_float_limits(1, 50);
			
	//--------------define all options for the STREAMLINE assignment---------------------------//	
			GLUI_Rollout* streamline_rollout = control_window->add_rollout("Streamline", true);
			control_window->add_checkbox_to_panel(streamline_rollout, "Draw streamline", &view.draw_streamline);
			GLUI_Spinner* streamline_size = control_window->add_spinner_to_panel(streamline_rollout, "Streamline size", GLUI_SPINNER_INT, &view.streamline_size);
			streamline_size->set_int_limits(2, 120);
			
	//--------------define the slices options--------------------------------------------------//
			GLUI_Rollout* slices_rollout = control_window->add_rollout("Slices", true);
			control_window->add_checkbox_to_panel(slices_rollout, "Draw slices", &view.draw_slices);
			GLUI_Spinner* n_slices = control_window->add_spinner_to_panel(slices_rollout, "N slices", GLUI_SPINNER_INT, &slice_size);
			n_slices->set_int_limits(1, 50);
		

			//--------------viewpoint options--------------------------------------------------//
			GLUI_Rollout* view_rollout = control_window->add_rollout("Viewpoint", true);

			GLUI_Spinner* ex = control_window->add_spinner_to_panel(view_rollout, "ex", GLUI_SPINNER_FLOAT, &view.ex);
			ex->set_speed(0.2);

			GLUI_Spinner* ey = control_window->add_spinner_to_panel(view_rollout, "ey", GLUI_SPINNER_FLOAT, &view.ey);
			ey->set_speed(0.2);

			GLUI_Spinner* ez = control_window->add_spinner_to_panel(view_rollout, "ez", GLUI_SPINNER_FLOAT, &view.ez);
			ez->set_speed(0.2);

			GLUI_Spinner* cx = control_window->add_spinner_to_panel(view_rollout, "cx", GLUI_SPINNER_FLOAT, &view.cx);
			cx->set_speed(0.2);

			GLUI_Spinner* cy = control_window->add_spinner_to_panel(view_rollout, "cy", GLUI_SPINNER_FLOAT, &view.cy);
			cy->set_speed(0.2);

			GLUI_Spinner* cz = control_window->add_spinner_to_panel(view_rollout, "cz", GLUI_SPINNER_FLOAT, &view.cz);
			cz->set_speed(0.2);
			

			GLUI_Spinner* ux = control_window->add_spinner_to_panel(view_rollout, "ux", GLUI_SPINNER_FLOAT, &view.ux);
			ux->set_speed(2);

			GLUI_Spinner* uy = control_window->add_spinner_to_panel(view_rollout, "uy", GLUI_SPINNER_FLOAT, &view.uy);
			uy->set_speed(2);

			GLUI_Spinner* uz = control_window->add_spinner_to_panel(view_rollout, "uz", GLUI_SPINNER_FLOAT, &view.uz);
			uz->set_speed(2);


		

	
	glutMainLoop();			//calls do_one_simulation_step, keyboard, display, drag, reshape
	return 0;
}

