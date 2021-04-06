#include<iostream>
#include<fstream>
#include<string.h>
#include"solver.h"
#include"util.h"
#include"MLEM.h"
#include"SART.h"
using namespace std;

char scheme[100] = "exp/shepp_logan.rec.sche";
char in_dir[100];
//char in_dir[100] = "ģ��ͶӰ����/ƽ���ƶ��켣/����";
char geo_cfg[100];
char check_point[100];
char out_file[100];
//char out_file[100] = "ly_82x400x400_uint8.rec_la1_it1.raw";
char tmp_str[100];
Config *cfg = new Config();

SART solver;

Config::raw_t* raw_data;

void open_file(ifstream& s, string fname) {
    if (s.is_open())s.close();
    s.open(fname);
    if (!s.is_open()) {
        cout << "opening file " << fname << " failed!" << endl;
        exit(-1);
    }
}

void save_sections_raw(bool k_first = false){
    int I = cfg->object_I, J = cfg->object_J, K = cfg->object_K;
    int IJ = I * J, JK = J * K;
    raw_data = new Config::raw_t[I*J*K];
    double maxv = 0;
    for (int v = 0; v < I * J * K; v++) {
        maxv = max(maxv, solver.voxel[v]);
    }
    int full = (1ll << (8 * sizeof(Config::raw_t))) - 1;
    for (int i = 0; i < I; i++) {
        for (int j = 0; j < J; j++) {
            for (int k = 0; k < K; k++) {
                int v = i * JK + j * K + k;
                int vo = v;
                if (k_first) {
                    vo = k * IJ + i * J + j;
                }
                raw_data[vo] = min(1.0, max(0.0,solver.voxel[v] / maxv)) * full;
            }
        }
    }

    ofstream fout(out_file,ios::binary);
    if(!fout.is_open()){
        printf("write file failed");
        exit(0);
    }
    fout.write((char*)raw_data,I*J*K*sizeof(Config::raw_t));

}


int main(){
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
        sprintf(tmp_str,"%s/projectImage%d.raw",in_dir,i+1);
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
    cout << sec << "s\n";
    for(int t=0;t<T;t++){
        clock_t clk = clock();
        cout<<"epoch "<<t+1<<"\n";
        solver.iterate();
        cout << "  ok\n";
        double e = timer(clk);
        sec += e;
        cout<<e<<"s (total " << sec << "s)\n";
        printf("time1 = %.2fs time2 = %.2fs\n", 1.0*solver.siddon.time1 / CLOCKS_PER_SEC,
            1.0*solver.siddon.time2 / CLOCKS_PER_SEC);
    }
    save_sections_raw();
}