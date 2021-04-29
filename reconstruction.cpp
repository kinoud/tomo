#include<iostream>
#include<fstream>
#include<string.h>
#include"solver.h"
#include"util.h"
#include"MLEM.h"
#include"SART.h"
using namespace std;

extern char* working_dir;
static const char* scheme = "rec.sche";
char in_dir[100];
static char geo_cfg[100];
char check_point[100];
char out_file[100];
static char tmp_str[100];
extern Config *cfg;

SART solver;

Config::raw_voxel_t* raw_data;

extern void open_file(ifstream& s, const char* fname);

void save_voxel(){
    int V = cfg->object_I * cfg->object_J * cfg->object_K;
    raw_data = new Config::raw_voxel_t[V];
    int full = (1ll << (8 * sizeof(Config::raw_voxel_t))) - 1;
    for (int v = 0; v < V; v++) {
        if (solver.voxel[v] > full) {
            printf("voxel %.2f cut down to limit %d (which makes a bad checkpoint)\n",solver.voxel[v],full);
        }
        else {
            raw_data[v] = solver.voxel[v];
        }
    }
    sprintf(tmp_str, "%s/%s", working_dir, out_file);
    ofstream fout(tmp_str,ios::binary);
    if(!fout.is_open()){
        printf("write file '%s' failed\n", tmp_str);
        exit(-1);
    }
    fout.write((char*)raw_data,V*sizeof(Config::raw_voxel_t));
}


void reconstruction(){
    ifstream fin;
    open_file(fin, scheme);
    int T;
    fin >> in_dir
        >> geo_cfg
        >> check_point
        >> out_file
        >> cfg->board_I
        >> cfg->board_J
        >> cfg->board_w
        >> cfg->object_I
        >> cfg->object_J
        >> cfg->object_K
        >> cfg->object_w
        >> cfg->object_h
        >> cfg->lambda
        >> T;
    // printf("geo=%s\n",geo_cfg);
    open_file(fin, geo_cfg);
    int n; fin >> n;
    for(int i=0;i<n;i++){
        p3 tube;
        fin >> tube.x >> tube.y >> tube.z;
        //cout << tube.x << " " << tube.y << " " << tube.z << " " << endl;
        cfg->tubes.push_back(tube);
        sprintf(tmp_str, "%s/%s/projectImage%d.raw", working_dir, in_dir, i + 1);
        cfg->projections.push_back(string(tmp_str));
    }
    for (int i = 0; i < n; i++) {
        p3 det;
        fin >> det.x >> det.y >> det.z;
        //cout << det.x << " " << det.y << " " << det.z << " " << endl;
        cfg->boards.push_back(det);
    }
    solver.init(cfg);
    if (strcmp(check_point, "none") != 0)
        solver.load_cp(check_point);
    //solver.read_proj(cfg->projections[0]);
    //save_raw(cfg, solver.proj, "test.raw");
    //return 0;
    double sec = 0;
    printf("%.1fs\n", sec);
    for(int t=0;t<T;t++){
        clock_t clk = clock();
        printf("epoch %d/%d\n", t + 1, T);
        solver.iterate();
        printf(" ok\n");
        double e = timer(clk);
        sec += e;
        printf("%.1fs (total %.1fs)\n", e, sec);
    }
    save_voxel();
}