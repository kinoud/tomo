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
using namespace std;

void mem_stat(int new_bytes)
{
    static long long bytes = 0;
    static int prog = 0;
    int step = 50; //MB
    bytes += new_bytes;
    if (bytes / (step * (1 << 20)) > prog)
    {
        prog = bytes / (step * (1 << 20));
        cout << "[mem " << prog * step << "MB allocated]" << endl;
    }
}

void SART::init(Config *cfg)
{
    this->cfg = cfg;
    view_k = 0;
    siddon.init(cfg);

    int bI = cfg->board_I, bJ = cfg->board_J;
    int V = cfg->object_I * cfg->object_J * cfg->object_K;
    delete[] voxel_factor;
    delete[] dvoxel;
    delete[] voxel;
    delete[] proj;
    delete[] sproj;
    delete[] line_weight;

    int nt = cfg->num_threads;
    proj = new double[bI * bJ];
    sproj = new double[bI * bJ];
    voxel_factor = new double[nt * V];
    dvoxel = new double[nt * V];
    voxel = new double[V];
    line_weight = new double[bI];
    mem_stat(2 * bI * bJ * sizeof(double));
    mem_stat((nt + nt + 1) * V * sizeof(double));
    mem_stat(bI*sizeof(double));

    for (int i = 0; i < V; i++)
        voxel[i] = 0;

    delete[] raw_data;
    raw_data = new Config::raw_t[bI * bJ];
    mem_stat(bI * bJ * sizeof(Config::raw_t));
}

void SART::fun(int x){
    printf("hi %d\n",x);
}

void SART::iterate()
{
    int nt = cfg->num_threads;
    int *Is = new int[nt + 1];
    thread *ths = new thread[nt];

    int bI = cfg->board_I, bJ = cfg->board_J;
    int V = cfg->object_I * cfg->object_J * cfg->object_K;
    Is[0] = 0,Is[nt]=bI;
    
    while (view_k < cfg->tubes.size())
    {
        //printf("\r                                                                     ");
        printf("\rview %2d/%d ",view_k+1,cfg->tubes.size());
        read_raw(view_k);
        for(int i=0;i<bI;i++)line_weight[i]=0;
        double W=0;
        for (int r = 0; r < bI * bJ; r++){
            proj[r] = raw_data[r];
            W+=proj[r];
            line_weight[r/bJ]+=proj[r];
        }
        W/=nt;
        int nt_k=1;
        for(int i=1;i<bI;i++){
            line_weight[i]+=line_weight[i-1];
            if(line_weight[i]>W){
                Is[nt_k++]=i;
                line_weight[i]=0;
                if(nt_k==nt)break;
            }
        }
        for (int i = 0; i < nt; i++)
        {
            // printf(" %4d",Is[i+1]-Is[i]);
            // clock_t t1 = clock();
            // update(Is[i], Is[i + 1], dvoxel + i * V, voxel_factor + i * V);
            // printf("(%2.2fs)",1.0*(clock()-t1)/CLOCKS_PER_SEC);
            ths[i]=thread(
                [this, Is,i,V]{
                    update(Is[i], Is[i + 1], this->dvoxel + i * V, this->voxel_factor + i * V);
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
        for (int v = 0; v < V; v++)
        {
            if (voxel_factor[v] == 0)
                continue;
            voxel[v] += cfg->lambda * dvoxel[v] / voxel_factor[v];
            if (voxel[v] < 0)
                voxel[v] = 0;
        }
        view_k++;
    }
    delete[] Is;
}

void SART::update(int I0, int I1, double *dvoxel, double *voxel_factor)
{
    int bI = cfg->board_I, bJ = cfg->board_J;
    int V = cfg->object_I * cfg->object_J * cfg->object_K;

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

    for (int i = I0; i < I1; i++)
    {
        for (int j = 0; j < bJ; j++)
        {
            int r = i * bJ + j;
            // simulate projecting
            siddon.trace_ray({board0.x + i * cfg->board_w,
                              board0.y + j * cfg->board_w,
                              board0.z},
                             tube);
            double s = 0;
            for (int k = 0; k < siddon.voxel_len; k++)
            {
                s += voxel[siddon.voxel_idx[k]] * siddon.voxel_a[k];
            }
            sproj[r] = s;
            if (siddon.voxel_len == 0)
                continue;
            // back projecting
            for (int k = 0; k < siddon.voxel_len; k++)
            {
                int v = siddon.voxel_idx[k];
                dvoxel[v] += (proj[r] - sproj[r]) * siddon.voxel_a[k] / siddon.ray_len;
                voxel_factor[v] += siddon.voxel_a[k];
            }
        }
    }
}