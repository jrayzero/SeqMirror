#ifndef QUANT
#define QUANT

int quant_ac4x4_normal(int **input_block, int **output_block, int **pos_scan, int block_x_pos);
int quant_dc4x4_normal(int **input_block, int **output_block, int **pos_scan);

#endif
