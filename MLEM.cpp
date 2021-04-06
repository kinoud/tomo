#include "MLEM.h"
#include<iostream>
#include<vector>
#define MAX(a,b) (a<b?b:a)
using namespace std;

void MLEM::init(Config* cfg) {
    this->cfg = cfg;
    siddon.init(cfg);

    int bI = cfg->board_I, bJ = cfg->board_J;
    int V = cfg->object_I * cfg->object_J * cfg->object_K;
    delete[]voxel_factor;
    delete[]mvoxel;
    delete[]voxel;

    proj = new double[bI * bJ];
    sproj = new double[bI * bJ];
    voxel_factor = new double[V];
    mvoxel = new double[V];
    voxel = new double[V];

    for (int i = 0; i < V; i++)
        voxel[i] = 100;

    delete[] raw_data;
    raw_data = new Config::raw_t[bI * bJ];
}


void MLEM::simulate(int view_k) {
    int I = cfg->board_I, J = cfg->board_J;
    read_raw(view_k);
    for (int r = 0; r < I * J; r++)
        proj[r] = raw_data[r];
    p3 tube = cfg->tubes[view_k];
    p3 board = cfg->boards[view_k];
    p3 board0 = { board.x - 0.5 * (I - 1) * cfg->board_w,
                    board.y - 0.5 * (J - 1) * cfg->board_w,
                    board.z }; // center of pixel (0,0)
    for (int i = 0; i < I; i++) {
        for (int j = 0; j < J; j++) {
            int r = i * J + j;
            // simulate projecting
            siddon.trace_ray({ board0.x + i * cfg->board_w,
                                board0.y + j * cfg->board_w,
                                board0.z }, tube);
            double s = 0;
            for (int k = 0; k < siddon.voxel_len; k++) {
                s += voxel[siddon.voxel_idx[k]] * siddon.voxel_a[k];
            }
            sproj[r] = s;
            if (siddon.voxel_len == 0)
                continue;
            // back projecting
            for (int k = 0; k < siddon.voxel_len; k++) {
                int v = siddon.voxel_idx[k];
                mvoxel[v] += siddon.voxel_a[k]*proj[r] / MAX(sproj[r],1.0);
                voxel_factor[v] += siddon.voxel_a[k];
            }
        }
    }
}

void MLEM::iterate() {
    
    int V = cfg->object_I * cfg->object_J * cfg->object_K;

    for (int i = 0; i < V; i++) {
        mvoxel[i] = 0;
        voxel_factor[i] = 0;
    }

    for (int view_k = 0; view_k < cfg->tubes.size(); view_k++) {
        printf("\rview %2d/%d", view_k + 1, cfg->tubes.size());
        simulate(view_k);
    }

    for (int v = 0; v < V; v++) {
        if (voxel_factor[v] == 0)continue;
        voxel[v] *= mvoxel[v] / voxel_factor[v];
    }
}