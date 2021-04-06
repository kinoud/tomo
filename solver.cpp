#include"solver.h"
#include<fstream>
#include<iostream>
using namespace std;

void Solver::read_raw(int view_k) {
    // printf("ccn=%p\n", cfg);
    ifstream fin(cfg->projections[view_k], ios::binary);
    if (!fin.is_open()) {
        cout << "open file failed" << endl;
        exit(0);
    }
    int bI = cfg->board_I, bJ = cfg->board_J;
    fin.read((char*)raw_data, bI * bJ * sizeof(Config::raw_t));
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