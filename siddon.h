#pragma once
#include "config.h"
#include<time.h>
class Siddon {
private:
	// parameters
	Config *cfg;

	// calculated parameters
	double object_x0, object_y0, object_z0;
	double object_x1 = 0, object_y1 = 0, object_z1 = 0;
	double object_w, object_h;
	int object_J, object_K, object_JK;

	// helper functions
	
	int pos_to_idx(double x, double y, double z);
	void generate_ts(int& k, double xbase, double w, double tube_x, double board_x, double t0, double t1);
	
	// tmp vars
	double* arr_t = NULL, * arr_t_tmp = NULL;

	
public:
	int* voxel_idx = NULL;
	double* voxel_a = NULL;
	double ray_len = 0;
	int voxel_len = 0;
	Siddon();
	~Siddon();
	void init(Config* cfg);
	void trace_ray(p3,p3);
	clock_t time1 = 0, time2 = 0, time3 = 0;
};