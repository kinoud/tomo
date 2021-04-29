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

extern char* working_dir;
static const char* scheme = "proj.sche";
char voxel_in[100];
static char geo_cfg[100];
char out_dir[100];
Config::raw_voxel_t* in_data;
Config::raw_proj_t* out_data;
double* voxel;
double* proj;

extern Config *cfg;
Siddon siddon;
double maxp=0;
long long full = (1ll << (sizeof(Config::raw_proj_t) * 8)) - 1;
static char tmp_str[100];

extern void open_file(ifstream& s, const char* fname);

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
			maxp = max(maxp, p);
		}
	}
	int IJ = cfg->board_I * cfg->board_J;
	for (int i = 0; i < IJ; i++) {
		if (proj[i] > full) {
			out_data[i] = full;
			printf("projection value %.2f cut down to limit %lld\n", proj[i], full);
		}
		else {
			out_data[i] = proj[i];
		}
	}
	ofstream fout(savefile, ios::binary);
	fout.write((char*)out_data, IJ * sizeof(Config::raw_proj_t));
}

void projection() {
	ifstream fin;
	open_file(fin, scheme);
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
	in_data = new Config::raw_voxel_t[V];
	out_data = new Config::raw_proj_t[cfg->board_I * cfg->board_J];
	open_file(fin, geo_cfg);
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
	
	sprintf(tmp_str, "%s/%s", working_dir, voxel_in);
	fin.close();
	fin.open(tmp_str, ios::binary);
	printf("opening file '%s' ...\n", tmp_str);
	if (!fin.is_open()) {
		printf("opening file '%s' failed\n");
		exit(-1);
	}
	clock_t t = clock();
	cout << "reading data...";
	fin.read((char*)in_data, sizeof(Config::raw_voxel_t) * V);
	double a = 0, b = 0;
	for (int v = 0; v < V; v++) {
		voxel[v] = in_data[v];
		a = max(a, voxel[v]);
		b = min(b, voxel[v]);
	}
	printf("maxv = %.2f, minv = %.2f\n", a, b);
	cout << "ok\n";
	fin.close();
	printf("%.2fs used\n", timer(t));
	
	sprintf(tmp_str, "%s/%s", working_dir, out_dir);
	sprintf(out_dir, "%s", tmp_str);
	int f = 1;
	while (_access(tmp_str, 0) == 0) {
		sprintf(tmp_str, "%s(%d)", out_dir, f++);
	}
	sprintf(out_dir, "%s", tmp_str);

	int flag = _mkdir(out_dir);
	if (flag) {
		printf("creating directory '%s' failed\n",out_dir);
		exit(-1);
	}
	t = clock();
	maxp = 0;
	for (int k = 0; k < cfg->tubes.size(); k++) {
		p3 tube = cfg->tubes[k];
		p3 board = cfg->boards[k];
		sprintf(tmp_str, "%s/projectImage%d.raw", out_dir, k + 1);
		proj_and_save(tube, board, tmp_str);
		printf("\rview %2d/%d", k + 1, cfg->tubes.size());
	}puts("");
	printf("maxv=%.2f (limit=%lld)\n", maxp, full);
	printf("%.2fs used\n", timer(t));
}