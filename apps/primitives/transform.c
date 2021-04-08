#include "common.h"
#include "transform.h"

void forward4x4(int *block, int *out_block,  int block_row, int block_col) {
  for (int i = block_row; i < block_row + 4; i++) {
    int *row = &block[i * MB_BLOCK_SIZE + block_col];
    int *row_out = &out_block[i * MB_BLOCK_SIZE + block_col];
    int p0 = row[0];
    int p1 = row[1];
    int p2 = row[2];
    int p3 = row[3];
    int t0 = p0 + p3;
    int t1 = p1 + p2;
    int t2 = p1 - p2;
    int t3 = p0 - p3;
    *row_out = t0 + t1;
    row_out++;
    *row_out = (t3 << 1) + t2;
    row_out++;
    *row_out = t0 - t1;
    row_out++;
    *row_out = t3 - (t2 << 1);
    
  }
  for (int i = block_col; i < block_col + 4; i++) {
    int p0 = out_block[block_row * MB_BLOCK_SIZE + i];
    int p1 = out_block[(block_row+1) * MB_BLOCK_SIZE + i];
    int p2 = out_block[(block_row+2) * MB_BLOCK_SIZE + i];
    int p3 = out_block[(block_row+3) * MB_BLOCK_SIZE + i];
    int t0 = p0 + p3;
    int t1 = p1 + p2;
    int t2 = p1 - p2;
    int t3 = p0 - p3;
    out_block[block_row * MB_BLOCK_SIZE + i] = t0 + t1;
    out_block[(block_row+1) * MB_BLOCK_SIZE + i] = t2 + (t3 << 1);
    out_block[(block_row+2) * MB_BLOCK_SIZE + i] = t0 - t1;
    out_block[(block_row+3) * MB_BLOCK_SIZE + i] = t3 - (t2 << 1);    
  }
}

// block is 16x16
// different function cause going from 16x16 (block, extract the dcs) to 4x4 (out_block)
void hadamard4x4_DCs(int *block, int *out_block) {
  for (int i = 0; i < 4; i++) {
    int *row = &block[(i*4) * MB_BLOCK_SIZE];
    int *row_out = &out_block[i * 4];
    int p0 = row[0];
    int p1 = row[4];
    int p2 = row[8];
    int p3 = row[12];
    int t0 = p0 + p3;
    int t1 = p1 + p2;
    int t2 = p1 - p2;
    int t3 = p0 - p3;
    *row_out = t0 + t1;
    row_out++;
    *row_out = t3 + t2;
    row_out++;
    *row_out = t0 - t1;
    row_out++;
    *row_out = t3 - t2;
    
  }
  for (int i = 0; i < 4; i++) {
    int p0 = out_block[i];
    int p1 = out_block[4 + i];
    int p2 = out_block[8 + i];
    int p3 = out_block[12 + i];
    int t0 = p0 + p3;
    int t1 = p1 + p2;
    int t2 = p1 - p2;
    int t3 = p0 - p3;
    out_block[i] = (t0 + t1) >> 1;
    out_block[4 + i] = (t2 + t3) >> 1;
    out_block[8 + i] = (t0 - t1) >> 1;
    out_block[12 + i] = (t3 - t2) >> 1;
  }
}

void ihadamard4x4_DCs(int *block, int *reconstruction) {
  for (int i = 0; i < BLOCK_SIZE; i++) {
    int *row = &block[i*4];
    int *row_out = &reconstruction[(i*4) * MB_BLOCK_SIZE];
    int t0 = row[0];
    int t1 = row[1];
    int t2 = row[2];
    int t3 = row[3];

    int p0 = t0 + t2;
    int p1 = t0 - t2;
    int p2 = t1 - t3;
    int p3 = t1 + t3;

    row_out[0] = p0 + p3;
    row_out[4] = p1 + p2;    
    row_out[8] = p1 - p2;
    row_out[12] = p0 - p3;
  }

  //  Vertical 
  for (int i = 0; i < MB_BLOCK_SIZE; i+=4) {
    int p0 = reconstruction[i];
    int p1 = reconstruction[i+4*MB_BLOCK_SIZE];
    int p2 = reconstruction[i+8*MB_BLOCK_SIZE];
    int p3 = reconstruction[i+12*MB_BLOCK_SIZE];

    int t0 = p0 + p2;
    int t1 = p0 - p2;
    int t2 = p1 - p3;
    int t3 = p1 + p3;

    reconstruction[i] = t0 + t3;
    reconstruction[i+4*MB_BLOCK_SIZE] = t1 + t2;
    reconstruction[i+8*MB_BLOCK_SIZE] = t1 - t2;
    reconstruction[i+12*MB_BLOCK_SIZE] = t0 - t3;
  }
}

void inverse4x4(int *in, int *out, int pos_y, int pos_x) {
  for (int i = pos_y; i < pos_y + 4; i++) {
    int *row = &in[i * MB_BLOCK_SIZE + pos_x];
    int *row_out = &out[i * MB_BLOCK_SIZE + pos_x];
    int p0 = row[0];
    int p1 = row[1];
    int p2 = row[2];
    int p3 = row[3];
    int t0 = p0 + p2;
    int t1 = p0 - p2;
    int t2 = (p1 >> 1) - p3;
    int t3 = p1 + (p3 >> 1);
    *row_out = t0 + t3;
    row_out++;
    *row_out = t1 + t2;
    row_out++;
    *row_out = t1 - t2;
    row_out++;
    *row_out = t0 - t3;
    
  }
  for (int i = pos_x; i < pos_x + 4; i++) {
    int p0 = out[pos_y * MB_BLOCK_SIZE + i];
    int p1 = out[(pos_y+1) * MB_BLOCK_SIZE + i];
    int p2 = out[(pos_y+2) * MB_BLOCK_SIZE + i];
    int p3 = out[(pos_y+3) * MB_BLOCK_SIZE + i];
    int t0 = p0 + p2;
    int t1 = p0 - p2;
    int t2 = (p1 >> 1) - p3;
    int t3 = p1 + (p3 >> 1);
    out[pos_y * MB_BLOCK_SIZE + i] = t0 + t3;
    out[(pos_y+1) * MB_BLOCK_SIZE + i] = t1 + t2;
    out[(pos_y+2) * MB_BLOCK_SIZE + i] = t1 - t2;
    out[(pos_y+3) * MB_BLOCK_SIZE + i] = t0 - t3;
  }
}


/*void _forward4x4(int **block, int **tblock, int pos_y, int pos_x)
{
  int i, ii;  
  int tmp[16];
  int *pTmp = tmp, *pblock;
  int p0,p1,p2,p3;
  int t0,t1,t2,t3;

  // Horizontal
  for (i=pos_y; i < pos_y + BLOCK_SIZE; i++)
  {
    pblock = &block[i][pos_x];
    p0 = *(pblock++);
    p1 = *(pblock++);
    p2 = *(pblock++);
    p3 = *(pblock  );

    t0 = p0 + p3;
    t1 = p1 + p2;
    t2 = p1 - p2;
    t3 = p0 - p3;

    *(pTmp++) =  t0 + t1;
    *(pTmp++) = (t3 << 1) + t2;
    *(pTmp++) =  t0 - t1;    
    *(pTmp++) =  t3 - (t2 << 1);
  }

  // Vertical 
  for (i=0; i < BLOCK_SIZE; i++)
  {
    pTmp = tmp + i;
    p0 = *pTmp;
    p1 = *(pTmp += BLOCK_SIZE);
    p2 = *(pTmp += BLOCK_SIZE);
    p3 = *(pTmp += BLOCK_SIZE);

    t0 = p0 + p3;
    t1 = p1 + p2;
    t2 = p1 - p2;
    t3 = p0 - p3;

    ii = pos_x + i;
    tblock[pos_y    ][ii] = t0 +  t1;
    tblock[pos_y + 1][ii] = t2 + (t3 << 1);
    tblock[pos_y + 2][ii] = t0 -  t1;
    tblock[pos_y + 3][ii] = t3 - (t2 << 1);
  }
}


void hadamard4x4(int **block, int **tblock)
{
  int i;
  int tmp[16];
  int *pTmp = tmp, *pblock;
  int p0,p1,p2,p3;
  int t0,t1,t2,t3;

  // Horizontal
  for (i = 0; i < BLOCK_SIZE; i++)
  {
    pblock = block[i];
    p0 = *(pblock++);
    p1 = *(pblock++);
    p2 = *(pblock++);
    p3 = *(pblock  );

    t0 = p0 + p3;
    t1 = p1 + p2;
    t2 = p1 - p2;
    t3 = p0 - p3;

    *(pTmp++) = t0 + t1;
    *(pTmp++) = t3 + t2;
    *(pTmp++) = t0 - t1;    
    *(pTmp++) = t3 - t2;
  }

  // Vertical 
  for (i = 0; i < BLOCK_SIZE; i++)
  {
    pTmp = tmp + i;
    p0 = *pTmp;
    p1 = *(pTmp += BLOCK_SIZE);
    p2 = *(pTmp += BLOCK_SIZE);
    p3 = *(pTmp += BLOCK_SIZE);

    t0 = p0 + p3;
    t1 = p1 + p2;
    t2 = p1 - p2;
    t3 = p0 - p3;

    tblock[0][i] = (t0 + t1) >> 1;
    tblock[1][i] = (t2 + t3) >> 1;
    tblock[2][i] = (t0 - t1) >> 1;
    tblock[3][i] = (t3 - t2) >> 1;
  }
}


void hadamard4x2(int **block, int **tblock)
{
  int i;
  int tmp[8];
  int *pTmp = tmp;
  int p0,p1,p2,p3;
  int t0,t1,t2,t3;

  // Horizontal
  *(pTmp++) = block[0][0] + block[1][0];
  *(pTmp++) = block[0][1] + block[1][1];
  *(pTmp++) = block[0][2] + block[1][2];
  *(pTmp++) = block[0][3] + block[1][3];

  *(pTmp++) = block[0][0] - block[1][0];
  *(pTmp++) = block[0][1] - block[1][1];
  *(pTmp++) = block[0][2] - block[1][2];
  *(pTmp  ) = block[0][3] - block[1][3];

  // Vertical
  pTmp = tmp;
  for (i=0;i<2;i++)
  {      
    p0 = *(pTmp++);
    p1 = *(pTmp++);
    p2 = *(pTmp++);
    p3 = *(pTmp++);

    t0 = p0 + p3;
    t1 = p1 + p2;
    t2 = p1 - p2;
    t3 = p0 - p3;

    tblock[i][0] = (t0 + t1);
    tblock[i][1] = (t3 + t2);
    tblock[i][2] = (t0 - t1);      
    tblock[i][3] = (t3 - t2);
  }
}

void ihadamard4x2(int **tblock, int **block)
{
  int i;  
  int tmp[8];
  int *pTmp = tmp;
  int p0,p1,p2,p3;
  int t0,t1,t2,t3;

  // Horizontal
  *(pTmp++) = tblock[0][0] + tblock[1][0];
  *(pTmp++) = tblock[0][1] + tblock[1][1];
  *(pTmp++) = tblock[0][2] + tblock[1][2];
  *(pTmp++) = tblock[0][3] + tblock[1][3];

  *(pTmp++) = tblock[0][0] - tblock[1][0];
  *(pTmp++) = tblock[0][1] - tblock[1][1];
  *(pTmp++) = tblock[0][2] - tblock[1][2];
  *(pTmp  ) = tblock[0][3] - tblock[1][3];

  // Vertical
  pTmp = tmp;
  for (i = 0; i < 2; i++)
  {
    p0 = *(pTmp++);
    p1 = *(pTmp++);
    p2 = *(pTmp++);
    p3 = *(pTmp++);

    t0 = p0 + p2;
    t1 = p0 - p2;
    t2 = p1 - p3;
    t3 = p1 + p3;

    // coefficients (transposed)
    block[0][i] = t0 + t3;
    block[1][i] = t1 + t2;
    block[2][i] = t1 - t2;
    block[3][i] = t0 - t3;
  }
}

//following functions perform 8 additions, 8 assignments. Should be a bit faster
void hadamard2x2(int **block, int tblock[4])
{
  int p0,p1,p2,p3;

  p0 = block[0][0] + block[0][4];
  p1 = block[0][0] - block[0][4];
  p2 = block[4][0] + block[4][4];
  p3 = block[4][0] - block[4][4];
 
  tblock[0] = (p0 + p2);
  tblock[1] = (p1 + p3);
  tblock[2] = (p0 - p2);
  tblock[3] = (p1 - p3);
}

void ihadamard2x2(int tblock[4], int block[4])
{
  int t0,t1,t2,t3;

  t0 = tblock[0] + tblock[1];
  t1 = tblock[0] - tblock[1];
  t2 = tblock[2] + tblock[3];
  t3 = tblock[2] - tblock[3];

  block[0] = (t0 + t2);
  block[1] = (t1 + t3);
  block[2] = (t0 - t2);
  block[3] = (t1 - t3);
}


void hadamard2x2(int **block, int tblock[4])
{
  //12 additions, 4 assignments
    tblock[0] = (block[0][0] + block[0][4] + block[4][0] + block[4][4]);
    tblock[1] = (block[0][0] - block[0][4] + block[4][0] - block[4][4]);
    tblock[2] = (block[0][0] + block[0][4] - block[4][0] - block[4][4]);
    tblock[3] = (block[0][0] - block[0][4] - block[4][0] + block[4][4]);
}

void ihadamard2x2(int tblock[4], int block[4])
{
    block[0] = (tblock[0] + tblock[1] + tblock[2] + tblock[3]);
    block[1] = (tblock[0] - tblock[1] + tblock[2] - tblock[3]);
    block[2] = (tblock[0] + tblock[1] - tblock[2] - tblock[3]);
    block[3] = (tblock[0] - tblock[1] - tblock[2] + tblock[3]);
}

*/
