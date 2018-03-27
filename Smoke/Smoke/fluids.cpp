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
controller_viewInteraction interaction;

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
		view.visualize( DIM, &model_fft, &color,&DENSITY, &VELOCITY, &FORCE, &dataset,
		&SCALAR_DENSITY, &SCALAR_VELOCITY, &SCALAR_FORCE, &dataset_scalar,
		&VECTOR_VELOCITY, &VECTOR_FORCE, &dataset_vector);
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



int main(int argc, char **argv)
{
	// Initialize models, view and controller
	model_fft = Model_fftw();
	color = Model_color();
	view = View_visualization();
	keyboard = Controller_keyboard();
	interaction = controller_viewInteraction();

	// Initialize the main visualization window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	int horizontal, vertical;
	GetDesktopResolution(&horizontal, &vertical);
	glutInitWindowSize(horizontal, vertical);

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
	control_window = GLUI_Master.create_glui_subwindow(main_window, GLUI_SUBWINDOW_RIGHT);

	

	//--------------define all options for the COLORMAPPING assignment---------------------------//
	GLUI_Rollout* color_rollout = control_window->add_rollout("Colors", true);

		control_window->add_checkbox_to_panel(color_rollout, "Draw smoke", &view.draw_smoke);
		control_window->add_checkbox_to_panel(color_rollout, "Clamp", &view.use_clamp);

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
		N_color_spinner->set_float_limits(2,255);

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

			GLUI_Panel* field_panel = control_window->add_panel_to_panel(glyph_rollout, "Glyph field");
			GLUI_Panel* type_panel = control_window->add_panel_to_panel(glyph_rollout, "Glyph type");
			GLUI_Panel* scalar_panel = control_window->add_panel_to_panel(glyph_rollout, "Scalar dataset");
			GLUI_Panel* vector_panel = control_window->add_panel_to_panel(glyph_rollout, "Vector dataset");

			GLUI_RadioGroup* field_buttons = control_window->add_radiogroup_to_panel(field_panel, &view.vector_type);
			GLUI_RadioGroup* type_buttons = control_window->add_radiogroup_to_panel(type_panel, &view.glyph_type);
			GLUI_RadioGroup* scalar_buttons = control_window->add_radiogroup_to_panel(scalar_panel, &dataset_scalar);
			GLUI_RadioGroup* vector_buttons = control_window->add_radiogroup_to_panel(vector_panel, &dataset_vector);

			control_window->add_radiobutton_to_group(field_buttons, "Standard");
			control_window->add_radiobutton_to_group(field_buttons, "Gradient");

			control_window->add_radiobutton_to_group(type_buttons, "Cones");
			control_window->add_radiobutton_to_group(type_buttons, "Arrows");

			control_window->add_radiobutton_to_group(scalar_buttons, "Density");
			control_window->add_radiobutton_to_group(scalar_buttons, "Velocity");
			control_window->add_radiobutton_to_group(scalar_buttons, "Force");

			control_window->add_radiobutton_to_group(vector_buttons, "Velocity");
			control_window->add_radiobutton_to_group(vector_buttons, "Force");


			GLUI_Spinner* N_sampleX_spinner = control_window->add_spinner_to_panel(glyph_rollout, "Row samples", GLUI_SPINNER_INT, &view.glyph_samplesY);
			N_sampleX_spinner->set_float_limits(1, 50);

			GLUI_Spinner* N_sampleY_spinner = control_window->add_spinner_to_panel(glyph_rollout, "Column samples", GLUI_SPINNER_INT, &view.glyph_samplesX);
			N_sampleY_spinner->set_float_limits(1, 50);
			
	//--------------define all options for the STREAMLINE assignment---------------------------//	
			GLUI_Rollout* streamline_rollout = control_window->add_rollout("Streamline", true);
			control_window->add_checkbox_to_panel(streamline_rollout, "Draw streamline", &view.draw_streamline);
			
	
	glutMainLoop();			//calls do_one_simulation_step, keyboard, display, drag, reshape
	return 0;
}

