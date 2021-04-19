#pragma once
#include"config.h"
#include"solver.h"
using namespace std;

class DDSART: public Solver{
private:
	int view_k;
	void update();
    
    void back_project(int vi,int vj,int vk);
    p3 board_base;
    p3 object_base;
    p3 tube;
	double* proj = NULL, * eproj = NULL;
public:
	void init(Config*cfg);
    void set_geo(p3 tube, p3 board);
	double project(int pi,int pj);
	virtual void iterate();
};