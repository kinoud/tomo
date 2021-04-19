#include<iostream>
#include<iomanip>
#include<fstream>
#include<time.h>
#include<io.h>
#include<direct.h>
#include"assert.h"
#include"siddon.h"
#include"util.h"
#include"DDSART.h"

using namespace std;

char scheme[100] = "exp/proj.sche";
char voxel_in[100];
char geo_cfg[100];
char out_dir[100];
Config::raw_t* in_data;
Config::raw_t* out_data;
double* proj;

Config *cfg = new Config();
DDSART solver;
double maxp=0;
char tmp_str[100];


void hprint(int i, int j) {
	printf("(%d,%d) %d\n", i, j, out_data[i * cfg->board_J + j]);
}

void proj_and_save(p3 tube, p3 board, char* savefile) {
	
	solver.set_geo(tube, board);
	int I = cfg->board_I, J = cfg->board_J;

	for (int i = 0; i < I; i++) {
		for (int j = 0; j < J; j++) {
			
			
			proj[i * J + j] = solver.project(i, j);
			
		}
	}
	

	if (savefile == NULL){
		for(int i=0;i<I;i++)
			for(int j=0;j<J;j++)
				maxp = max(maxp,proj[i*J+j]);
		return;
	}

	int IJ = I * J;
	int full = (1ll << (sizeof(Config::raw_t) * 8)) - 1;
	
	for (int i = 0; i < IJ; i++)
		out_data[i] = min(1.0, proj[i] / maxp) * full;

	ofstream fout(savefile, ios::binary);
	fout.write((char*)out_data, IJ * sizeof(Config::raw_t));
}

int main() {
	ifstream fin;
	fin.open(scheme);
	assert(fin.is_open());
	fin >> voxel_in 
		>> geo_cfg
		>> out_dir
		>> cfg->board_I
		>> cfg->board_J
		>> cfg->board_w
		>> cfg->object_I
		>> cfg->object_J
		>> cfg->object_K
		>> cfg->object_w
		>> cfg->object_h;
	int V = cfg->object_I * cfg->object_J * cfg->object_K;
	in_data = new Config::raw_t[V];
	out_data = new Config::raw_t[cfg->board_I * cfg->board_J];
	fin.close();
	fin.open(geo_cfg);
	assert(fin.is_open());
	int n; fin >> n;
	for (int i = 0; i < n; i++) {
		p3 tube;
		fin >> tube.x >> tube.y >> tube.z;
		cfg->tubes.push_back(tube);
	}
	for (int i = 0; i < n; i++) {
		p3 det;
		fin >> det.x >> det.y >> det.z;
		cfg->boards.push_back(det);
	}
	solver.init(cfg);
	proj = new double[cfg->board_I * cfg->board_J];
	
	fin.close();

	clock_t t = clock();
	cout << "reading data...";
	solver.load_cp(voxel_in);
	cout << "ok\n";
	printf("%.2fs used\n", timer(t));
	
	sprintf(tmp_str, "%s", out_dir);
	int f = 1;
	while (_access(tmp_str, 0) == 0) {
		sprintf(tmp_str, "%s(%d)", out_dir, f++);
	}
	sprintf(out_dir, "%s", tmp_str);
	int flag = _mkdir(out_dir);
	if (flag) {
		cout << "create directory failed\n";
		exit(-1);
	}
	t = clock();
	maxp = 0;
	for (int step = 0; step <= 1; step++) {
		for (int k = 0; k < cfg->tubes.size(); k++) {
			p3 tube = cfg->tubes[k];
			p3 board = cfg->boards[k];
			if (step == 0) {
				proj_and_save(tube, board, NULL);
				printf("\r(1/2)view %2d/%d", k + 1, cfg->tubes.size());
			}
			else {
				sprintf(tmp_str, "%s/projectImage%d.raw", out_dir, k + 1);
				proj_and_save(tube, board, tmp_str);
				printf("\r(2/2)view %2d/%d", k + 1, cfg->tubes.size());
			}
		}puts("");
	}
	printf("%.2fs used\n", timer(t));
}