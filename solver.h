#pragma once
#include"siddon.h"
#include<string>
#include<vector>
class Solver
{
protected:
	Config* cfg;
	Config::raw_proj_t* raw_data = NULL;
	void read_raw(int view_k, Config::raw_proj_t*dst=NULL);
public:
	double* voxel = NULL;
	vector<double> differences;
	virtual void init(Config* cfg);
	virtual void iterate() = 0;
	virtual double project(int, int) = 0;
	virtual void set_geo(p3,p3) = 0;
	void load_cp(char* raw_file_name);
};

