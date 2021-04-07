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

void inverse4x4(int **tblock, int **block, int pos_y, int pos_x)
{
  int i, ii;  
  int tmp[16];
  int *pTmp = tmp, *pblock;
  int p0,p1,p2,p3;
  int t0,t1,t2,t3;

  // Horizontal
  for (i = pos_y; i < pos_y + BLOCK_SIZE; i++)
  {
    pblock = &tblock[i][pos_x];
    t0 = *(pblock++);
    t1 = *(pblock++);
    t2 = *(pblock++);
    t3 = *(pblock  );

    p0 =  t0 + t2;
    p1 =  t0 - t2;
    p2 = (t1 >> 1) - t3;
    p3 =  t1 + (t3 >> 1);

    *(pTmp++) = p0 + p3;
    *(pTmp++) = p1 + p2;
    *(pTmp++) = p1 - p2;
    *(pTmp++) = p0 - p3;
  }

  //  Vertical 
  for (i = 0; i < BLOCK_SIZE; i++)
  {
    pTmp = tmp + i;
    t0 = *pTmp;
    t1 = *(pTmp += BLOCK_SIZE);
    t2 = *(pTmp += BLOCK_SIZE);
    t3 = *(pTmp += BLOCK_SIZE);

    p0 = t0 + t2;
    p1 = t0 - t2;
    p2 =(t1 >> 1) - t3;
    p3 = t1 + (t3 >> 1);

    ii = i + pos_x;
    block[pos_y    ][ii] = p0 + p3;
    block[pos_y + 1][ii] = p1 + p2;
    block[pos_y + 2][ii] = p1 - p2;
    block[pos_y + 3][ii] = p0 - p3;
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

void ihadamard4x4(int **tblock, int **block)
{
  int i;  
  int tmp[16];
  int *pTmp = tmp, *pblock;
  int p0,p1,p2,p3;
  int t0,t1,t2,t3;

  // Horizontal
  for (i = 0; i < BLOCK_SIZE; i++)
  {
    pblock = tblock[i];
    t0 = *(pblock++);
    t1 = *(pblock++);
    t2 = *(pblock++);
    t3 = *(pblock  );

    p0 = t0 + t2;
    p1 = t0 - t2;
    p2 = t1 - t3;
    p3 = t1 + t3;

    *(pTmp++) = p0 + p3;
    *(pTmp++) = p1 + p2;
    *(pTmp++) = p1 - p2;
    *(pTmp++) = p0 - p3;
  }

  //  Vertical 
  for (i = 0; i < BLOCK_SIZE; i++)
  {
    pTmp = tmp + i;
    t0 = *pTmp;
    t1 = *(pTmp += BLOCK_SIZE);
    t2 = *(pTmp += BLOCK_SIZE);
    t3 = *(pTmp += BLOCK_SIZE);

    p0 = t0 + t2;
    p1 = t0 - t2;
    p2 = t1 - t3;
    p3 = t1 + t3;
    
    block[0][i] = p0 + p3;
    block[1][i] = p1 + p2;
    block[2][i] = p1 - p2;
    block[3][i] = p0 - p3;
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
