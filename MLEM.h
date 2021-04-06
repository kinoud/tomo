#pragma once
#include "solver.h"
class MLEM: public Solver
{
private:
	double* mvoxel = NULL;
	double* voxel_factor = NULL;
	double* proj = NULL, * sproj = NULL;
	void simulate(int view_k);
public:
	virtual void init(Config* cfg);
	virtual void iterate();
};

