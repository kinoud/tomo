#include<iostream>
#include<iomanip>
#include<fstream>
#include<time.h>
#include<io.h>
#include<direct.h>
#include"assert.h"
#include"siddon.h"
#include"util.h"

using namespace std;

char scheme[100] = "exp/proj.sche";
char voxel_in[100];
char geo_cfg[100];
char out_dir[100];
typedef Config::raw_t raw_in_t;
typedef Config::raw_t raw_out_t;
raw_in_t* in_data;
raw_out_t* out_data;
double* voxel;
double* proj;

Config *cfg = new Config();
Siddon siddon;
double maxp=0;
char tmp_str[100];

void proj_and_save(p3 tube, p3 board, char* savefile) {
	p3 board0 = { board.x - 0.5 * (cfg->board_I - 1) * cfg->board_w,
					board.y - 0.5 * (cfg->board_J - 1) * cfg->board_w,
					board.z }; // center of pixel (0,0)
	for (int i = 0; i < cfg->board_I; i++) {
		for (int j = 0; j < cfg->board_J; j++) {
			siddon.trace_ray({ board0.x + i * cfg->board_w,
								board0.y + j * cfg->board_w,
								board0.z }, tube);
			double p = 0;
			for (int k = 0; k < siddon.voxel_len; k++) {
				assert(siddon.voxel_a[k] >= 0);
				assert(voxel[siddon.voxel_idx[k]] >= 0);
				p += siddon.voxel_a[k] * voxel[siddon.voxel_idx[k]];
			}
			assert(p >= 0);
			proj[i * cfg->board_J + j] = p;
			assert(proj[i * cfg->board_J + j] >= 0);
			if (savefile != NULL) { maxp = max(maxp, p); }
		}
	}
	if (savefile == NULL)return;
	int IJ = cfg->board_I * cfg->board_J;
	int full = (1ll << (sizeof(raw_out_t) * 8)) - 1;
	for (int i = 0; i < IJ; i++) {
		out_data[i] = min(1.0, proj[i] / maxp) * full;
	}
	ofstream fout(savefile, ios::binary);
	fout.write((char*)out_data, IJ * sizeof(raw_out_t));
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
	in_data = new raw_in_t[V];
	out_data = new raw_out_t[cfg->board_I * cfg->board_J];
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
	siddon.init(cfg);
	voxel = new double[cfg->object_I * cfg->object_J * cfg->object_K];
	proj = new double[cfg->board_I * cfg->board_J];
	
	fin.close();
	fin.open(voxel_in,ios::binary);
	if (!fin.is_open()) {
		cout << "open file failed" << endl;
		exit(0);
	}
	clock_t t = clock();
	cout << "reading data...";
	fin.read((char*)in_data, sizeof(raw_in_t) * V);
	double a = 0, b = 0;
	for (int v = 0; v < V; v++) {
		voxel[v] = in_data[v];
		a = max(a, voxel[v]);
		b = min(b, voxel[v]);
	}
	printf("maxv = %.2f, minv = %.2f\n", a, b);
	cout << "ok\n";
	printf("%.2fs used\n", timer(t));
	
	if (_access(out_dir, 0) == 0) {
		int flag = _rmdir(out_dir);
		if (flag) {
			cout << "delete directory failed\n";
			exit(-1);
		}
	}
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
				printf("\r(1)view %2d/%d", k + 1, cfg->tubes.size());
			}
			else {
				sprintf(tmp_str, "%s/projectImage%d.raw", out_dir, k + 1);
				proj_and_save(tube, board, tmp_str);
				printf("\r(2)view %2d/%d", k + 1, cfg->tubes.size());
			}
		}puts("");
	}
	printf("%.2fs used\n", timer(t));
}