#ifndef COMMON
#define COMMON

#define BLOCK_SIZE 4
#define MB_BLOCK_SIZE 16
#define MB_BLOCK_SHIFT 4
#define MAX_PLANE 3
#define Q_BITS 15
#define bit_depth_y 8
#define bit_depth_c 8
#define max_imgpel_value 255
#define QP 28


// FOR QP == 28
#define iscale 256

#define offset_comp 174592
// DC
//
// AC
//int scale_comp_AC[4][4] = {8192, 5243, 8192, 5243, 5243, 3355, 5243, 3355, 8192, 5243, 8192, 5243, 5243, 3355, 5243, 3355};

#ifdef DEBUG
#define DO(foo) foo;
#else
#define DO(foo) 
#endif

int clip3(int x, int y, int z);
int clip1y(int x);

#endif

