#pragma once
#include<string>
#include<vector>
using namespace std;
struct p3{
    double x,y,z;
};

class Config{
public:
    int board_I,board_J,object_I,object_J,object_K;
    double board_w,object_w,object_h,
        lambda=1;
    typedef unsigned short raw_t;
    vector<p3> tubes;
    vector<p3> boards;
    vector<string> projections;
    Config(){}
};