#include<iostream>
#include<algorithm>
#include"assert.h"
#include"siddon.h"
#include<time.h>
#include<math.h>
using namespace std;

int fcmp(double x) {
	if (abs(x) < 1e-6)return 0;
	return x > 0 ? 1 : -1;
}
inline int Siddon::pos_to_idx(double x, double y, double z) {
	int i = (int)((x - object_x0) / object_w);
	int j = (int)((y - object_y0) / object_w);
	int k = (int)((z - object_z0) / object_h);
	return i * object_JK + j * object_K + k;
}
void Siddon::generate_ts(int& k, double xbase, double w, double tube_x, double board_x, double t0, double t1) {
	double x0 = tube_x + t0 * (board_x - tube_x);
	double x;
	if (board_x >= tube_x) {
		x = xbase + ceil((x0 - xbase) / w) * w;
	}
	else {
		x = xbase + floor((x0 - xbase) / w) * w;
	}
	double t = (x - tube_x) / (board_x - tube_x);
	assert(fcmp(t - t0) >= 0);
	double dt = abs(w / (board_x - tube_x));
	int n = 0, m = 0;
	while (t - t1 < 0) {
		while (n < k && arr_t[n] - t < 0) {
			arr_t_tmp[m++] = arr_t[n++];
		}
		if (n < k && arr_t[n] - t>0)
			arr_t_tmp[m++] = t;
		t += dt;
	}
	while (n < k)
		arr_t_tmp[m++] = arr_t[n++];
	k = m;
	swap(arr_t, arr_t_tmp);
}

Siddon::Siddon() {}
Siddon::~Siddon() {
	delete[]arr_t;
	delete[]voxel_idx;
	delete[]voxel_a;
}

void Siddon::init(Config* cfg) {
	this->cfg = cfg;
	int n = cfg->object_I + cfg->object_J + cfg->object_K;
	object_K = cfg->object_K;
	object_JK = cfg->object_J * object_K;
	object_w = cfg->object_w;
	object_h = cfg->object_h;
	delete[]arr_t;
	delete[]voxel_idx;
	delete[]voxel_a;
	voxel_idx = new int[n];
	voxel_a = new double[n];
	arr_t = new double[3 * n];
	arr_t_tmp = new double[3 * n];
	object_x0 = -0.5 * cfg->object_I * cfg->object_w;
	object_y0 = -0.5 * cfg->object_J * cfg->object_w;
	object_z0 = -0.5 * cfg->object_K * cfg->object_h;
	object_x1 = -object_x0;
	object_y1 = -object_y0;
	object_z1 = -object_z0;
}
void Siddon::trace_ray(p3 det, p3 tube) {
	p3 d = { det.x - tube.x,det.y - tube.y,det.z - tube.z };
	double t_z0, t_z1, t_x0, t_x1, t_y0, t_y1;

	t_x0 = (object_x0 - tube.x) / d.x;
	t_x1 = (object_x1 - tube.x) / d.x;
	t_y0 = (object_y0 - tube.y) / d.y;
	t_y1 = (object_y1 - tube.y) / d.y;
	t_z0 = (object_z0 - tube.z) / d.z;
	t_z1 = (object_z1 - tube.z) / d.z;

	if (t_x0 > t_x1) { swap(t_x0, t_x1); }
	if (t_y0 > t_y1) { swap(t_y0, t_y1); }
	if (t_z0 > t_z1) { swap(t_z0, t_z1); }

	double t0 = max(t_x0, max(t_y0, t_z0));
	double t1 = min(t_x1, min(t_y1, t_z1));
	if (t0>=t1) {
		voxel_len = 0;
		ray_len =0;
		return;
	}

	clock_t clk = clock();

	int k = 0;
	arr_t[k++] = t0; arr_t[k++] = t1;
	generate_ts(k, object_x0, cfg->object_w, tube.x, det.x, t0, t1);
	generate_ts(k, object_y0, cfg->object_w, tube.y, det.y, t0, t1);
	generate_ts(k, object_z0, cfg->object_h, tube.z, det.z, t0, t1);

	time1 += clock() - clk;
	clk = clock();

	ray_len = sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
	int m = 0;
	for (int i = 0; i < k - 1; i++) {
		double len = (arr_t[i + 1] - arr_t[i]) * ray_len;
		if (len < 1e-5) {// mm
			continue;
		}
		voxel_a[m] = len;
		double t = (arr_t[i + 1] + arr_t[i]) * 0.5;
		voxel_idx[m] = pos_to_idx(tube.x + t * d.x, tube.y + t * d.y, tube.z + t * d.z);
		m++;
	}
	voxel_len = m;
	time2 += clock() - clk;
}

