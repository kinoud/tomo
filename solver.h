#pragma once
#include"siddon.h"
#include<string>
#include<vector>
class Solver
{
protected:
	Config* cfg;
	Config::raw_t* raw_data = NULL;
	void read_raw(int view_k, Config::raw_t *dst=NULL);
public:
	vector<Siddon> siddons;
	double* voxel = NULL;
	void init(Config* cfg);
	virtual void iterate()=0;
	void load_cp(char* raw_file_name);
};

