#include<iostream>
#include<fstream>
#include<string.h>
#include"solver.h"
#include"util.h"
#include"MLEM.h"
#include"SART.h"
using namespace std;

extern char* working_dir;
static const char* scheme = "task.sche";
char in_dir[100];
static char geo_cfg[100];
char check_point[100];
char out_file[100];
static double* voxel=NULL;
static char tmp_str[100];
extern Config *cfg;
extern Solver* solver;

Config::raw_voxel_t* raw_data;

extern void open_file(ifstream& s, const char* fname);

void save_voxel(ofstream& fout){
    int V = cfg->object_I * cfg->object_J * cfg->object_K;
    raw_data = new Config::raw_voxel_t[V];
    int full = (1ll << (8 * sizeof(Config::raw_voxel_t))) - 1;
    for (int v = 0; v < V; v++) {
        if (solver->voxel[v] > full) {
            printf("voxel %.2f cut down to limit %d (which makes a bad checkpoint)\n",solver->voxel[v],full);
        }
        else {
            raw_data[v] = solver->voxel[v];
        }
    }
    
    
    fout.write((char*)raw_data,V*sizeof(Config::raw_voxel_t));
}


void reconstruction(){
    ifstream fin;
    open_file(fin, scheme);
    int T;
    int zres_factor;
    fin >> tmp_str
        >> geo_cfg
        >> in_dir
        >> out_file
        >> check_point
        >> zres_factor
        >> cfg->lambda
        >> T
        >> cfg->board_I
        >> cfg->board_J
        >> cfg->board_w
        >> cfg->object_I
        >> cfg->object_J
        >> cfg->object_K
        >> cfg->object_w
        >> cfg->object_h;
    cfg->object_h *= zres_factor;
    cfg->object_K /= zres_factor;
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
    solver->init(cfg);
    if (strcmp(check_point, "none") != 0) {
        sprintf(tmp_str, "%s/%s", working_dir, check_point);
        solver->load_cp(tmp_str);
    }
    //solver->read_proj(cfg->projections[0]);
    //save_raw(cfg, solver->proj, "test.raw");
    //return 0;

    sprintf(tmp_str, "%s/%s", working_dir, out_file);
    printf("opening file '%s' for writing...\n", tmp_str);
    ofstream fout(tmp_str, ios::binary);
    if (!fout.is_open()) {
        printf("write file '%s' failed\n", tmp_str);
        exit(-1);
    }
    sprintf(tmp_str, "%s/%s.log", working_dir, out_file);
    printf("opening file '%s' for logging...\n", tmp_str);
    ofstream flog(tmp_str);
    if (!flog.is_open()) {
        printf("write file '%s' failed\n", tmp_str);
        exit(-1);
    }

    delete voxel;
    int V = cfg->object_I * cfg->object_J * cfg->object_K;
    voxel = new double[V];
    memcpy(voxel, solver->voxel, V * sizeof(double));
    double sec = 0;
    printf("%.1fs\n", sec);
    double saved_avgd = 1e10;
    for(int t=0;t<T;t++){
        clock_t clk = clock();
        printf("epoch %d/%d\n", t + 1, T);
        flog << "epoch " << t + 1 << "/" << T << '\n';
        flog << "lambda = " << cfg->lambda << '\n';
        solver->iterate();
        printf(" ok\n");
        double e = timer(clk);
        sec += e;
        printf("%.1fs used (total %.1fs)\n", e, sec);

        double voxel_diff = 0, VE0 = 0;
        for (int v = 0; v < V; v++) {
            voxel_diff += (voxel[v] - solver->voxel[v]) * (voxel[v] - solver->voxel[v]);
            VE0 += voxel[v] * voxel[v];
        }
        voxel_diff = sqrt(voxel_diff / VE0);
        memcpy(voxel, solver->voxel, V * sizeof(double));

        double maxd = 0, mind = solver->differences[0], avgd = 0;
        for (double d : solver->differences) {
            maxd = max(maxd, d);
            mind = min(mind, d);
            avgd += d;
        }
        avgd /= solver->differences.size();
        cfg->lambda *= 0.99;
        saved_avgd = avgd;
        sprintf(tmp_str,"proj RMSE max min avg = %.3f%% %.3f%% %.3f%%\n", 100*maxd, 100*mind, 100*avgd);
        flog << tmp_str;
        printf("%s",tmp_str);
        sprintf(tmp_str,"voxel RMSE            = %.3f%%\n", 100*voxel_diff);
        flog << tmp_str << '\n';
        printf("%s",tmp_str);
        puts("");
    }
    save_voxel(fout);
}