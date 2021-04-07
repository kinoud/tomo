#pragma once
#include "solver.h"
#include<mutex>
using namespace std;
class MLEM: public Solver
{
private:
	double* _mvoxel = NULL;
	double* _voxel_factor = NULL;
	double* _proj = NULL;
	mutex _mtx;
	void simulate(int th);
public:
	void init(Config* cfg);
	virtual void iterate();
};

