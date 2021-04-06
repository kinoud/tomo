#pragma once
#include"config.h"
#include"siddon.h"
#include"solver.h"
#include<string>
struct int_double{
    int x;
    double y;
};

class SART: public Solver{
private:
	double *dvoxel=NULL;
	double *voxel_factor=NULL;
	int view_k;
	Config *cfg;
	
public:
	Siddon siddon;
	double* proj = NULL, * sproj = NULL;
	double* voxel = NULL;
	virtual void init(Config*cfg);
	void update();
	virtual void iterate();
};