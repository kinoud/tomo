# 基于C++的层析合成迭代重建

[TOC]

## 配置项类

配置项被封装在Config类中。

**公有成员变量**

- `int board_I` 投影图像像素行数。
- `int board_J` 投影图像像素列数。
- `int object_I` 重建体素每层体素行数。
- `int object_J` 重建体素每层体素列数。
- `int object_K` 重建体素层数。
- `double board_w` 投影图像像素边长（毫米）。
- `double object_w` 体素边长（毫米）。
- `double object_h` 体素高（毫米）。
- `double lambda` SART松弛因子。
- `int num_threads` 多线程数目。
- `vector<p3> tubes` 光源轨迹。
- `vector<p3> boards` 与光源轨迹对应的探测器中心移动轨迹。
- `vector<string> projections` 投影图像路径数组。

## Solver类

Solver类是重建算法的基类，不同的重建算法派自此类。此类对投影、重建等功能进行了封装。

**公有成员变量**

- `double* voxel` 体素值数组。

**公有成员函数**

- `void init(Config* cfg)` 初始化函数，参数为指向配置对象的指针。
- `void set_geo(p3 tube,p3 board)` 设置几何关系，`tube`和`board`分别是涉嫌光源坐标和探测器板中心坐标。
- `double project(int pi,int pj)` 对像素`(pi,pj`进行正投影，返回投影值。
- `void load_cp(char* filename)` 载入体素初始值数据到体素值数组中。
- `void iterate()` 进行一轮迭代更新。

Solver类是一个抽象类，按照使用的算法不同派生子类，子类有：

- SART：基于Siddon正投影的联合代数迭代重建算法。
- DDSART：基于一种近似体积正投影算法的SART算法。
- MLEM：基于Siddon正投影的最大似然估计统计迭代算法。

初始化一个Solver类前需要先定义好配置类对象。一个典型的初始化过程如下：

```c++
Config* cfg = new Config();
/*...此处省略对配置项赋值的过程...*/
Solver solver = new SART();
solver->init(cfg);
```

## 投影

投影过程可以由Solver类实现。

一个典型的投影过程如下：

```c++
// 配置几何关系
solver.set_geo(tube,board);
// 遍历每个像素进行投影
for(int i=0;i<I;i++){
   for(int j=0;j<J;j++){
       projection[i][j]=solver.project(i,j);
   } 
}
```

## 重建

重建过程可以由Solver类实现。

一个典型的重建过程如下：

```c++
// 进行T轮迭代重建
for(int t=0;t<T;t++){
    solver.iterate();
}
```

## 实验驱动程序

我们编写了一套完整的投影、重建驱动程序，程序入口是exp.cpp中的`main`函数。

exp.cpp中使用硬编码的方式给出了3个配置项：

1. `int task` 表示任务类型（值1表示投影，值2表示重建）。
2. `char* working_dir` 工作目录。
3. `Solver* solver` 一个实例化的Solver子类。

要使该程序正确运行，需要在工作目录文件夹下保存一个名为task.sche的文本文件（后缀名为sche）。

task.sche配置了投影/重建需要的全部参数，参数罗列如下：

```bash
投影用体素数据文件
几何关系文件
投影图片保存文件夹/重建用投影图片文件夹
重建保存结果文件
重建cp
重建z分辨率比例系数
lambda
迭代次数
board_I
board_J
board_w
object_I
object_J
object_K
object_w
object_h
```

上述参数在task.sche中以空格分开，同一参数内部不可以有空格。对上述参数的解释详见配置项类一节。

几何关系文件是一个文本文件，首先给出投影张数，然后按顺序给出全部的光源轨迹坐标，然后按顺序给出全部的探测器中心轨迹坐标。空格分开。

投影图片文件夹中的投影图片名称为projectImageX.raw，X是从1开始的连续自然数，表示投影图片序号。图片为raw格式，小端存储，数据类型32位无符号整数，行优先。

体素数据文件为raw格式，小端存储，数据类型为16位无符号整数，数组优先顺序为：`voxel[object_I][object_J][object_K]`。



## 其他类

### Siddon类

Siddon类实现了Siddon正投影算法。

**公有成员变量**

- `int* voxel_idx` 一次光线追踪过程中射线经过的体素索引数组，用来保存此结果。
- `double* voxel_a` 一次光线追踪过程中射线与经过体素的交线段长度数组，用来保存此结果。
- `double ray_len` 一次光线追踪过程中射线穿过全体体素的长度。
- `int voxel_len` 一次光线追踪过程中射线经过的体素数目。

**公有成员函数**

- `init(Config* cfg)` 使用配置项指针初始化。
- `void trace_ray(p3 to, p3 from)` 光源从`from`出发射到`to`，对此过程进行光线追踪。

### SART类

SART类是抽象类Solver的一个实现。实现了多线程加速的基于Siddon正投影算法的SART迭代重建。接口与Solver类保持一致。

### DDSART类

DDSART类是抽象类Solver的一个实现。实现了多线程加速的基于一种近似体积投影算法的SART迭代重建。接口与Solver类保持一致。

### MLEM类

MLEM类是抽象类MLEM的一个实现。实现了多线程加速的基于Siddon正投影算法的MLEM迭代重建。接口与Solver类保持一致。

## 程序清单

**头文件**

- config.h
- DDSART.h
- MLEM.h
- SART.h
- siddon.h
- solver.h
- util.h

**源码文件**

- exp.cpp
- projection.cpp
- reconstruction.cpp
- siddon.cpp
- solver.cpp
- SART.cpp
- DDSART.cpp
- MLEM.cpp

**可视化/数据处理等脚本文件**

- geo.ipynb 生成、可视化几何关系配置
- vis.ipynb 重建结果、投影图像可视化等
- voxel.ipynb 生成模拟体素数据、变化体素数据等
- real.ipynb 预处理真实数据