#pragma once
#include <rfftw.h>   
class Model_fftw
{
	public:
		Model_fftw();
		void init_simulation(int n);
		void FFT(int direction, void* vx);
		void solve(int n);
		void diffuse_matter(int n);
		int clamp(float x);
		void set_forces(int n);
		double dt;				//simulation time step
		float visc;				//fluid viscosity
		fftw_real *vx, *vy;             //(vx,vy)   = velocity field at the current moment
		fftw_real *vx0, *vy0;           //(vx0,vy0) = velocity field at the previous moment
		fftw_real *fx, *fy;	            //(fx,fy)   = user-controlled simulation forces, steered with the mouse 
		fftw_real *rho, *rho0;			//smoke density at the current (rho) and previous (rho0) moment 
		rfftwnd_plan plan_rc, plan_cr;  //simulation domain discretization
	};

