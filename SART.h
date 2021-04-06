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
	double *line_weight=NULL;
	void fun(int x);
	void update(int I0,int I1,double* dvoxel,double* voxel_factor);
	
public:
	double* proj = NULL, * sproj = NULL;
	virtual void init(Config*cfg);
	
	virtual void iterate();
};