#include <iostream>
#include <iomanip>
#include <vector>
#include <thread>
#include <functional>
#include "SART.h"
#include "config.h"
#include "siddon.h"
#include "assert.h"
#include "util.h"
#include <fstream>
#include<mutex>
#include<time.h>
using namespace std;

void SART::init(Config *cfg)
{
    Solver::init(cfg);
    siddons.resize(cfg->num_threads);
    for (int i = 0; i < cfg->num_threads; i++)
        siddons[i].init(cfg);
    view_k = 0;
    srand(time(0));
    int bI = cfg->board_I, bJ = cfg->board_J;
    int V = cfg->object_I * cfg->object_J * cfg->object_K;
    delete[] voxel_factor;
    delete[] dvoxel;
    delete[] proj;
    delete[] _sta;
    delete[] turn_to_view;
    //delete[] is_bg;

    int nt = cfg->num_threads;
    proj = new double[bI * bJ];
    voxel_factor = new double[nt * V];
    dvoxel = new double[nt * V];
    p_cnt = new int[bI];
    _sta = new int[bI];
    
    turn_to_view = new int[cfg->tubes.size()];
    for (int i = 0; i < cfg->tubes.size(); i++)
        turn_to_view[i] = i;

    for (double& d : differences)
        d = 1e20;

    for (int i = 0; i < V; i++)
        voxel[i] = 0;
}

void SART::set_geo(p3 tube, p3 board) {
    this->tube = tube;
    this->board = board;
    board0 = { board.x - 0.5 * (cfg->board_I - 1) * cfg->board_w,
                    board.y - 0.5 * (cfg->board_J - 1) * cfg->board_w,
                    board.z }; // center of pixel (0,0)
}
double SART::project(int pi, int pj) {
    Siddon& siddon = siddons[0];

    siddon.trace_ray({ board0.x + pi * cfg->board_w,
                        board0.y + pj * cfg->board_w,
                        board0.z }, tube);
    double p = 0;
    for (int k = 0; k < siddon.voxel_len; k++) {
        p += siddon.voxel_a[k] * voxel[siddon.voxel_idx[k]];
    }
    return p;
}

void SART::iterate()
{
    int nt = cfg->num_threads;
    thread *ths = new thread[nt];

    int bI = cfg->board_I, bJ = cfg->board_J;
    int V = cfg->object_I * cfg->object_J * cfg->object_K;
    int turn = 0;

    int view_N = cfg->tubes.size();
    for (int i = 0; i < view_N;i++) {
        int a = rand() % view_N;
        int b = rand() % view_N;
        swap(turn_to_view[a], turn_to_view[b]);
    }

    double E0 = 0;
    view_k = 0;
    int worse_cnt = 0;
    printf("lambda = %.3f\n", cfg->lambda);
    while (turn < view_N)
    {
        view_k = turn_to_view[turn];
        differences[view_k] = 0;
        //printf("\r                                                                     ");
        printf("\rturn %2d/%d", turn + 1, view_N, view_k);
        read_raw(view_k);

        int W=0;
        for (int i = 0; i < bI; i++) {
            bool has = false;
            for (int j = 0; j < bJ; j++) {
                int r = i * bJ + j;
                proj[r] = raw_data[r];
                E0 += proj[r] * proj[r];
                if (raw_data[r] > 0)has = true;
            }
            if (has) {
                W++;
                p_cnt[i] = 1;
            }
            else p_cnt[i] = 0;
        }

        W/=(nt*50);
        _top = 0;
        _sta[_top++] = 0;
        for(int i=1;i<bI;i++){
            p_cnt[i]+=p_cnt[i-1];
            if(p_cnt[i]>W){
                _sta[_top++] = i;
                p_cnt[i]=0;
            }
        }
        _sta[_top++] = bI;

        for (int i = 0; i < nt; i++)
        {
            ths[i]=thread(
                [this,i]{
                    update(i);
                }
            );
        }
        for (int i = 0; i < nt; i++)
        {
            ths[i].join();
        }
        for (int i = 1; i < nt; i++)
        {
            for (int v = 0; v < V; v++)
            {
                dvoxel[v] += dvoxel[v + i * V];
                voxel_factor[v] += voxel_factor[v + i * V];
            }
        }
        int maxv = (1ll << (sizeof(Config::raw_voxel_t) * 8)) - 1;
        for (int v = 0; v < V; v++)
        {
            if (voxel_factor[v] == 0) {
                //printf("error: voxel_factor=0\n");
                continue;
            }
                
            voxel[v] += cfg->lambda * dvoxel[v] / voxel_factor[v];
            if (voxel[v] < 0)
                voxel[v] = 0;
            if (voxel[v] > maxv)
                voxel[v] = maxv;
        }
        differences[view_k] = sqrt(differences[view_k]/E0);
        turn++;
    }

    delete[] ths;
}

void SART::update(int th)
{
    int bI = cfg->board_I, bJ = cfg->board_J;
    int V = cfg->object_I * cfg->object_J * cfg->object_K;

    double* dvoxel = this->dvoxel + th * V;
    double* voxel_factor = this->voxel_factor + th * V;
    Siddon& siddon = siddons[th];

    p3 tube = cfg->tubes[view_k];
    p3 board = cfg->boards[view_k];

    for (int i = 0; i < V; i++)
    {
        dvoxel[i] = 0;
        voxel_factor[i] = 0;
    }

    p3 board0 = {board.x - 0.5 * (cfg->board_I - 1) * cfg->board_w,
                 board.y - 0.5 * (cfg->board_J - 1) * cfg->board_w,
                 board.z}; // center of pixel (0,0)
    double diff = 0;
    while (1) {
        int I0, I1;
        {
            unique_lock<mutex> lk(_mtx);
            if (_top<=1)break;
            I0 = _sta[_top - 2];
            I1 = _sta[_top - 1];
            _top--;
        }
        //printf("th%d[%d,%d)\n",th, I0, I1);
        for (int i = I0; i < I1; i++)
        {
            for (int j = 0; j < bJ; j++)
            {
                int r = i * bJ + j;
                //if (raw_data[r] == 0)continue;
                // simulate projecting
                siddon.trace_ray({ board0.x + i * cfg->board_w,
                                  board0.y + j * cfg->board_w,
                                  board0.z },
                    tube);
                double s = 0;
                for (int k = 0; k < siddon.voxel_len; k++)
                {
                    s += voxel[siddon.voxel_idx[k]] * siddon.voxel_a[k];
                }
                diff += (s - proj[r]) * (s - proj[r]);
                if (siddon.voxel_len == 0)
                    continue;
                // back projecting
                for (int k = 0; k < siddon.voxel_len; k++)
                {
                    int v = siddon.voxel_idx[k];
                    dvoxel[v] += (proj[r] - s) * siddon.voxel_a[k] / siddon.ray_len;
                    voxel_factor[v] += siddon.voxel_a[k];
                }
            }
        }
    }
    {
        unique_lock<mutex> lk(_mtx);
        this->differences[view_k] += diff;
    }
}