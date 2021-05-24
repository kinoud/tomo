#include"DDSART.h"

#include <thread>

#define MIN(x,y) (x<y?x:y)
#define MAX(x,y) (x<y?y:x)

void DDSART::init(Config* cfg){
    Solver::init(cfg);
    int I = cfg->board_I, J = cfg->board_J;
    proj = new double[I*J];
    eproj = new double[I*J];
    I = cfg->object_I, J = cfg->object_J;
    int K = cfg->object_K;
    object_base = {
        -I * 0.5 * cfg->object_w,
        -J * 0.5 * cfg->object_w,
        -K * 0.5 * cfg->object_h
    };
}


void DDSART::set_geo(p3 tube, p3 board){
    this->tube = tube;
    board_base = { 
        board.x - 0.5 * cfg->board_I * cfg->board_w,
		board.y - 0.5 * cfg->board_J * cfg->board_w,
		board.z 
    };
}
double DDSART::project(int pi, int pj) {
    double w = cfg->object_w, h = cfg->object_h;
    int I = cfg->object_I, J = cfg->object_J, K = cfg->object_K;

    p3 corner = { board_base.x + pi * cfg->board_w,board_base.y + pj * cfg->board_w,board_base.z };
    p3 corner2 = { corner.x + cfg->board_w, corner.y + cfg->board_w, corner.z };

    p2 d1 = {
        (tube.x - corner.x) / (tube.z - corner.z) * h,
        (tube.y - corner.y) / (tube.z - corner.z) * h
    };

    p2 d2 = {
        (tube.x - corner2.x) / (tube.z - corner2.z) * h,
        (tube.y - corner2.y) / (tube.z - corner2.z) * h
    };

    double z0 = object_base.z + 0.5 * h;
    double t0 = (z0 - tube.z) / (corner.z - tube.z);

    p2 c1 = {
        tube.x + t0 * (corner.x - tube.x) - object_base.x - d1.x,
        tube.y + t0 * (corner.y - tube.y) - object_base.y - d1.y
    };
    p2 c2 = {
        tube.x + t0 * (corner2.x - tube.x) - object_base.x - d2.x,
        tube.y + t0 * (corner2.y - tube.y) - object_base.y - d2.y
    };

    double dx0 = (c2.x - c1.x);
    double dy0 = (c2.y - c1.y);

    double s = 0, sum_a = 0;

    for (int k = 0; k < K; k++) {
        c1.x += d1.x, c1.y += d1.y;
        c2.x += d2.x, c2.y += d2.y;
        sum_a += (c2.y - c1.y) * (c2.x - c1.x);
        if (c1.x > w * I || c1.y > w * J || c2.x < 0 || c2.y < 0)
            continue;
        double rect = (c2.y - c1.y) * (c2.x - c1.x);
        double min_x = min(w, c2.x - c1.x);
        double min_y = min(w, c2.y - c1.y);
        int i0 = floor(MAX(0.0, c1.x) / w);
        int j0 = floor(MAX(0.0, c1.y) / w);
        double x = i0 * w, y0 = j0 * w, y;
        for (int i = i0; x < c2.x && i < I; i++, x += w) {
            double dx = MIN(MIN(min_x, x + w - c1.x), c2.x - x);
            y = y0;
            for (int j = j0; y < c2.y && j < J; j++, y += w) {
                int v = i * J * K + j * K + k;
                double a = dx * MIN(MIN(min_y, y + w - c1.y), c2.y - y);
                // printf("a=%.2f\n", a);
                s += voxel[v] * a;
            }
        }
    }
    int r = pi*cfg->board_J+pj;
    eproj[r] = (proj[r]-s)/sum_a;

    return s;
}

void DDSART::back_project(int vi,int vj,int vk){
    double w = cfg->object_w, h = cfg->object_h;
    
    // corner 和 corner2 是体素中截面正方形的两个角
    p3 corner = {
        object_base.x + vi*w,
        object_base.y + vj*w,
        object_base.z + (vk+0.5)*h
    };
    p3 corner2 = {
        corner.x + w,
        corner.y + w,
        corner.z
    };
    double t = (board_base.z - tube.z) / (corner.z - tube.z);
    p2 c1 = {
        tube.x + t*(corner.x-tube.x) - board_base.x,
        tube.y + t*(corner.y-tube.y) - board_base.y
    };
    p2 c2 = {
        tube.x + t*(corner2.x-tube.x) - board_base.x,
        tube.y + t*(corner2.y-tube.y) - board_base.y
    };
    
    // c1.x = max(0.0,c1.x);
    // c1.y = max(0.0,c1.y);
    
    w = cfg->board_w;
    int I = cfg->board_I, J = cfg->board_J;

    double sum_a = 0, eback = 0;
    double min_x = min(w, c2.x - c1.x);
    double min_y = min(w, c2.y - c1.y);
    int i0 = MAX(0,floor(c1.x/w));
    int j0 = MAX(0,floor(c1.y/w));
    double x = i0 * w, y0 = j0 * w, y;
    for (int i = i0; x < c2.x && i < I; i++, x += w) {
        y = y0;
        double dx = MIN(MIN(min_x, x + w - c1.x), c2.x - x);
        for (int j = j0; y < c2.y && j < J; j++, y += w) {
            int r = i*J+j;
            double a = dx * MIN(MIN(min_y, y + w - c1.y), c2.y - y);
            eback += eproj[r]*a;
        }
    }

    sum_a = (c2.y - c1.y) * (c2.x - c1.x);
    eback /= sum_a;
    //if (sum_a < 1e-8) eback = 0;
    //if(sum_a!=0)printf("%.5f\n", sum_a);
    int v = vi*cfg->object_J*cfg->object_K+vj*cfg->object_K+vk;
    voxel[v]+=eback;
    if(voxel[v]<0)
        voxel[v]=0;
}


void DDSART::update1(int th) {
    int nt = cfg->num_threads;
    for (int i = th; i < cfg->board_I; i+=nt) {
        for (int j = 0; j < cfg->board_J; j++) {
            project(i, j);
        }
    }
}

void DDSART::update2(int th) {
    int nt = cfg->num_threads;
    for (int i = th; i < cfg->object_I; i+=nt) {
        for (int j = 0; j < cfg->object_J; j++) {
            for (int k = 0; k < cfg->object_K; k++) {
                back_project(i, j, k);
            }
        }
    }
}

void DDSART::iterate(){
    int nt = cfg->num_threads;
    thread* ths = new thread[nt];

    for (view_k = 0; view_k < cfg->tubes.size(); view_k++) {
        printf("\rview %2d/%2d", view_k+1, cfg->tubes.size());
        read_raw(view_k);
        for (int r = 0; r < cfg->board_I * cfg->board_J; r++)
            proj[r] = raw_data[r];
        set_geo(cfg->tubes[view_k], cfg->boards[view_k]);
        for (int i = 0; i < nt; i++)
        {
            ths[i] = thread(
                [this, i] {
                    update1(i);
                }
            );
        }
        for (int i = 0; i < nt; i++)
        {
            ths[i].join();
        }
        for (int i = 0; i < nt; i++)
        {
            ths[i] = thread(
                [this, i] {
                    update2(i);
                }
            );
        }
        for (int i = 0; i < nt; i++)
        {
            ths[i].join();
        }
    }
    puts("");

    delete[] ths;
}