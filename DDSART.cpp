#include"DDSART.h"


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
    p3 corner2 = { corner.x + w, corner.y + w, corner.z };


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
        if (c1.x > w * I || c1.y > w * J || c2.x < 0 || c2.y < 0)
            continue;
        int i0 = floor(max(0.0, c1.x) / w);
        int j0 = floor(max(0.0, c1.y) / w);
        double x = i0 * w, y0 = j0 * w, y;
        for (int i = i0; i < I && x < c2.x; i++, x += w) {
            double dx = min(min(min(w, c2.x - c1.x), x + w - c1.x), c2.x - x);
            y = y0;
            for (int j = j0; j < J && y < c2.y; j++, y += w) {
                int v = i * J * K + j * K + k;
                double a = dx * min(min(min(w, c2.y - c1.y), y + w - c1.y), c2.y - y);
                s += voxel[v] * a;
                sum_a += a;
            }
        }

    }
    int r = pi*cfg->board_J+pj;
    eproj[r] = (proj[r]*w*w-s)/sum_a;
    return s;
}

void DDSART::back_project(int vi,int vj,int vk){
    double w = cfg->object_w, h = cfg->object_h;
    
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
    double t = (corner.z - tube.z)/(board_base.z - tube.z);
    p2 c1 = {
        tube.x + t*(corner.x-tube.x) - board_base.x,
        tube.y + t*(corner.y-tube.y) - board_base.y
    };
    p2 c2 = {
        tube.x + t*(corner2.x-tube.x) - board_base.x,
        tube.y + t*(corner2.y-tube.y) - board_base.y
    };
    
    c1.x = max(0.0,c1.x);
    c1.y = max(0.0,c1.y);
    
    w = cfg->board_w;
    int I = cfg->board_I, J = cfg->board_J;

    double sum_a = 0, eback = 0;
    int i0 = floor(c1.x/w);
    int j0 = floor(c1.y/w);
    double x=i0*w, y=j0*w;
    for(int i=i0;i<I&&x<c2.x;i++,x+=w){
        for(int j=j0;j<J&&y<c2.y;j++,y+=w){
            int r = i*J+j;
            double a = min(w,c2.x-x)*min(w,c2.y-y);
            sum_a += a;
            eback += eproj[r]*a;
        }
    }
    eback /= sum_a;
    int v = vi*cfg->object_J*cfg->object_K+vj*cfg->object_K+vk;
    voxel[v]+=eback;
    if(voxel[v]<0)
        voxel[v]=0;
}

void DDSART::iterate(){

}