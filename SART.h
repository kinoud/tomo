#pragma once
#include"config.h"
#include"siddon.h"
#include"solver.h"
#include<string>
#include<mutex>
#include<queue>
using namespace std;
struct int_double{
    int x;
    double y;
};

class SART: public Solver{
private:
	double *dvoxel=NULL;
	double *voxel_factor=NULL;
	int view_k;
	int *p_cnt=NULL;
	mutex _mtx;
	int* _sta=NULL;
	int _top;
	void update(int th);
	int* turn_to_view = NULL;
	vector<Siddon> siddons;
	p3 tube, board, board0;
public:
	double* proj = NULL;
	virtual void init(Config*cfg);
	virtual void set_geo(p3 tube, p3 board);
	virtual double project(int pi, int pj);
	virtual void iterate();
};