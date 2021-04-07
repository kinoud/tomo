#include"solver.h"
#include<fstream>
#include<iostream>
#include"util.h"
using namespace std;

void Solver::read_raw(int view_k, Config::raw_t *dst) {
    // printf("ccn=%p\n", cfg);
    if (dst == NULL)dst = raw_data;
    ifstream fin(cfg->projections[view_k], ios::binary);
    if (!fin.is_open()) {
        cout << "open file failed" << endl;
        exit(0);
    }
    int bI = cfg->board_I, bJ = cfg->board_J;
    fin.read((char*)dst, bI * bJ * sizeof(Config::raw_t));
}

void Solver::load_cp(char* raw_file_name) {
    ifstream fin(raw_file_name);
    if (!fin.is_open()) {
        printf("open file %s failed\n", raw_file_name);
        exit(0);
    }
    printf("loading voxel cp from %s (make sure it is IJK order)\n", raw_file_name);
    int V = cfg->object_I * cfg->object_J*cfg->object_K;
    Config::raw_t* cp = new Config::raw_t[V];
    fin.read((char*)cp, V * sizeof(Config::raw_t));
    
    for (int v = 0; v < V; v++)voxel[v] = cp[v];
    
    delete[] cp;
}

void Solver::init(Config* cfg) {
    this->cfg = cfg;
    siddons.resize(cfg->num_threads);
    for (int i = 0; i < cfg->num_threads; i++)
        siddons[i].init(cfg);
    int bI = cfg->board_I, bJ = cfg->board_J;
    int V = cfg->object_I * cfg->object_J * cfg->object_K;
    delete[]voxel;
    voxel = new double[V];
    mem_stat(V * sizeof(double));
    delete[] raw_data;
    raw_data = new Config::raw_t[bI * bJ];
    mem_stat(bI * bJ * sizeof(Config::raw_t));
}