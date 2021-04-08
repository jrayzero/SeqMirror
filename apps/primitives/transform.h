#ifndef TRANSFORM 
#define TRANSFORM

void forward4x4(int *block, int *out_block, int pos_y, int pos_x);
void inverse4x4(int *block, int *out_block, int pos_y, int pos_x);
void hadamard4x4_DCs(int *block, int *out_block);
void ihadamard4x4_DCs(int *block, int *reconstruction);
//void inverse4x4(int **tblock, int **block, int pos_y, int pos_x);
//void hadamard4x4(int **block, int **tblock);
//void ihadamard4x4(int **tblock, int **block);
//void hadamard4x2(int **block, int **tblock);
//void ihadamard4x2(int **tblock, int **block);
//void hadamard2x2(int **block, int tblock[4]);
//void ihadamard2x2(int tblock[4], int block[4]);

#endif
