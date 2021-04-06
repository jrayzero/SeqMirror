#ifndef COMMON
#define COMMON

#define BLOCK_SIZE 4
#define MB_BLOCK_SIZE 16
#define MB_BLOCK_SHIFT 4
#define imagepel int32_t
#define MAX_PLANE 3
#define Q_BITS = 15
#define bit_depth_y = 8
#define bit_depth_c = 8
#define max_imgpel_value = 255

// FOR QP == 28
#define iscale = 256
//int qp_per_matrix[51] = {0,0,0,0,0,0,1,1,1,1,1,1,2,2,2,2,2,2,3,3,3,3,3,3,4,4,4,4,4,4,5,5,5,5,5,5,6,6,6,6,6,6,7,7,7,7,7,7,8,8,8};
#define offset_comp = 174592
// DC
//int scale_comp_DC[4][4] = {8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192};
// AC
//int scale_comp_AC[4][4] = {8192, 5243, 8192, 5243, 5243, 3355, 5243, 3355, 8192, 5243, 8192, 5243, 5243, 3355, 5243, 3355};

#ifdef DEBUG
#define DO(foo) foo
#else
#define DO(foo) 
#endif

#endif
