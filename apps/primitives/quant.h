#ifndef QUANT
#define QUANT

#include <stdbool.h>

//int quant_ac4x4_normal(int **input_block, int **output_block, int **pos_scan, int block_x_pos);
bool quant_dc4x4_normal(int *block, int qp);
bool quant_ac4x4_normal(int *block, int qp, int i, int j);
void inverse_quant_DCs(int *recons, int qp);
#endif
