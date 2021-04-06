#ifndef TRANSFORM 
#define TRANSFORM

void forward4x4(int **block, int **tblock, int pos_y, int pos_x);
void inverse4x4(int **tblock, int **block, int pos_y, int pos_x);
void hadamard4x4(int **block, int **tblock);
void ihadamard4x4(int **tblock, int **block);
void hadamard4x2(int **block, int **tblock);
void ihadamard4x2(int **tblock, int **block);
void hadamard2x2(int **block, int tblock[4]);
void ihadamard2x2(int tblock[4], int block[4]);
void forward8x8(int **block, int **tblock, int pos_y, int pos_x);
void inverse8x8(int **tblock, int **block, int pos_x);

#endif
