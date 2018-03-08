#include "controller_viewInteraction.h"
#include "Model_fftw.h"
#include "View_visualization.h"

controller_viewInteraction::controller_viewInteraction()
{

}
// drag: When the user drags with the mouse, add a force that corresponds to the direction of the mouse
//       cursor movement. Also inject some new matter into the field at the mouse location.
void controller_viewInteraction::drag(View_visualization* view,Model_fftw* model_fft, int DIM, int* mx, int* my)
{
	int xi, yi, X, Y; double  dx, dy, len;
	static int lmx = 0, lmy = 0;				//remembers last mouse location

												// Compute the array index that corresponds to the cursor location 
	xi = (int)model_fft->clamp((double)(DIM + 1) * ((double)*mx / (double)view->winWidth));
	yi = (int)model_fft->clamp((double)(DIM + 1) * ((double)(view->winHeight - *my) / (double)view->winHeight));

	X = xi; Y = yi;

	if (X > (DIM - 1))  X = DIM - 1; if (Y > (DIM - 1))  Y = DIM - 1;
	if (X < 0) X = 0; if (Y < 0) Y = 0;

	// Add force at the cursor location 
	*my = view->winHeight - *my;
	dx = *mx - lmx; dy = *my - lmy;
	len = sqrt(dx * dx + dy * dy);
	if (len != 0.0) { dx *= 0.1 / len; dy *= 0.1 / len; }
	model_fft->fx[Y * DIM + X] += dx;
	model_fft->fy[Y * DIM + X] += dy;
	model_fft->rho[Y * DIM + X] = 10.0f;
	lmx = *mx; lmy = *my;
}
void controller_viewInteraction::reshape(View_visualization* view,int* w, int* h)
{
	glViewport(0.0f, 0.0f, (GLfloat)*w, (GLfloat)*h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, (GLdouble)*w, 0.0, (GLdouble)*h);
	view->winWidth = *w; view->winHeight = *h;

}
void controller_viewInteraction::setX(int x, View_visualization* view)
{
	view->MOUSEx = x;
}
void controller_viewInteraction::setY(int y, View_visualization* view)
{
		//parameters
	 view->MOUSEy = y;
}
void controller_viewInteraction::mouse(int* btn, int* state, int* x, int* y, View_visualization* view)
{
	if (*btn == GLUT_LEFT_BUTTON && *state == GLUT_DOWN)
	{
		view->draw_steamline = 1;
		setX(*x,&*view);
		setY(*y,&*view);
		//drawSquare(MOUSEx,HEIGHT-MOUSEy);
		glutPostRedisplay();

		printf("The Coordinate of the selected locations:\n");
		printf("==========================================\n");
		printf("X = %d\nY= %d\n", *x, *y);
		printf("\n%f, %f", view->MOUSEx, view->MOUSEy);
		printf("==========================================\n");

		float x2 = (float)*x;
		float y2 = (float)*y;
		GLfloat radius = 3.0;
		
	}
	if (*btn == GLUT_RIGHT_BUTTON && *state == GLUT_DOWN)
	{
		exit(1);   // To Exit the Program
	}
}
