#pragma once
#include<time.h>
#include<iostream>
#include<string>
#include<fstream>
#include"config.h"
using namespace std;

inline double timer(clock_t start) {
	return double(clock() - start) / CLOCKS_PER_SEC;
}

static char* uint8_proj=NULL;
inline void save_raw(Config*cfg,float* proj, string name, bool newly=false) {
	if (uint8_proj == NULL||newly) {
		delete[] uint8_proj;
		uint8_proj = new char[cfg->board_I * cfg->board_J];
	}
	
	float max_p = 0;
	for (int i = 0; i < cfg->board_I * cfg->board_J; i++)
		if (proj[i] > max_p)max_p = proj[i];
	//cout << "max=" << max_p << endl;
	for (int i = 0; i < cfg->board_I * cfg->board_J; i++)
		uint8_proj[i] = (int)(proj[i] / max_p * 255);
	ofstream fout(name, ios::binary);
	fout.write(uint8_proj, cfg->board_I * cfg->board_J);
}
