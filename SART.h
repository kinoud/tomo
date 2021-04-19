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
public:
	double* proj = NULL;
	void init(Config*cfg);
	
	virtual void iterate();
};