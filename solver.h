#pragma once
#include"siddon.h"
#include<string>
class Solver
{
protected:
	Config* cfg;
	Config::raw_t* raw_data = NULL;
	void read_raw(int view_k);
public:
	Siddon siddon;
	double* voxel = NULL;
	virtual void init(Config* cfg)=0;
	virtual void iterate()=0;
	void load_cp(char* raw_file_name);
};

