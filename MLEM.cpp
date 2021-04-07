#include "MLEM.h"
#include"util.h"
#include<iostream>
#include<vector>
#include<thread>
#include<mutex>
#define MAX(a,b) (a<b?  b:a)
using namespace std;

void MLEM::init(Config* cfg) {
    Solver::init(cfg);
    
    int bI = cfg->board_I, bJ = cfg->board_J;
    int V = cfg->object_I * cfg->object_J * cfg->object_K;
    delete[]_proj;
    delete[]_voxel_factor;
    delete[]_mvoxel;

    int nt = cfg->num_threads;
    
    _proj = new double[nt * bI * bJ];
    _voxel_factor = new double[nt * V];
    _mvoxel = new double[nt * V];
    
    mem_stat((bI * bJ * 2 * nt + 2 * V * nt) * sizeof(double));
    for (int i = 0; i < V; i++)
        voxel[i] = 100;
}


void MLEM::simulate(int th) {
    int I = cfg->board_I, J = cfg->board_J;
    int V = cfg->object_I * cfg->object_J * cfg->object_K;
    Siddon& siddon = siddons[th];
    double* proj = _proj + th * I * J;
    double* mvoxel = _mvoxel + th * V;
    double* voxel_factor = _voxel_factor + th * V;
    int view_k = th;
    
    while (view_k<cfg->tubes.size()) {
        {
            unique_lock<mutex> lk(_mtx);
            read_raw(view_k);
            for (int r = 0; r < I * J; r++)
                proj[r] = raw_data[r];
        }
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
                // back projecting
                for (int k = 0; k < siddon.voxel_len; k++) {
                    int v = siddon.voxel_idx[k];
                    mvoxel[v] += siddon.voxel_a[k] * proj[r] / MAX(s, 1.0);
                    voxel_factor[v] += siddon.voxel_a[k];
                }
            }
        }
        printf("=");
        view_k += cfg->num_threads;
    }    
}

void MLEM::iterate() {

    int V = cfg->object_I * cfg->object_J * cfg->object_K;

    double* mvoxel = _mvoxel;
    double* voxel_factor = _voxel_factor;
    for(int i=0;i<cfg->tubes.size();i++)
        printf("_");
    puts("");
    int nt = cfg->num_threads;
    for (int i = 0; i < nt*V; i++) {
        mvoxel[i] = 0;
        voxel_factor[i] = 0;
    }
    thread* ths = new thread[nt];
    for (int i = 0; i < nt; i++) {
        ths[i] = thread(
            [this, i] {
                simulate(i);
            }
        );
    }
    for (int i = 0; i < nt; i++) {
        ths[i].join();
    }

    for (int i = 1; i < nt; i++) {
        for (int v = 0; v < V; v++) {
            mvoxel[v] += mvoxel[v + V * i];
            voxel_factor[v] += voxel_factor[v + V * i];
        }
    }

    for (int v = 0; v < V; v++) {
        if (voxel_factor[v] == 0)continue;
        voxel[v] *= mvoxel[v] / voxel_factor[v];
    }
    delete[] ths;
}