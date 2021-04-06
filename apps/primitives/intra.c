#include <stdlib.h
#include "common.h"
#include "intra.h"
#include "transform.h"

void JM_intra16x16_luma_vertical(int **pred_res, int *ref_pels)
{
  int j;
  for (j = 0; j < MB_BLOCK_SIZE; j++)
    memcpy(pred_res[j], &ref_pels[1], MB_BLOCK_SIZE * sizeof(int));
}

/*
void JM_intra16x16_luma_horizontal(int **pred_res, int *ref_pels)
{
  int i, j;
  int *prediction = &ref_pels[16];

  for (j = 0; j < MB_BLOCK_SIZE; j++)
  {  
    prediction++;
    for (i = 0; i < MB_BLOCK_SIZE; i++)
    {
      pred_res[j][i]  = *prediction;
    }    
  }
}

void JM_intra16x16_luma_dc(int **pred_res, int *ref_pels, int left_available, int up_available)
{
  int i, j, s0 = 0, s1 = 0, s2 = 0;

  if (up_available)
  {
    for (i = 1; i < MB_BLOCK_SIZE + 1; ++i)
      s1 += ref_pels[i];
  }

  if (left_available)
  {
    for (i = 17; i < 33; ++i)
      s2 += ref_pels[i];    // sum vert pix
  }

  if (up_available)
  {
    s0 = left_available
      ? rshift_rnd_sf((s1 + s2),(MB_BLOCK_SHIFT + 1)) // no edge
      : rshift_rnd_sf(s1, MB_BLOCK_SHIFT);          // left edge
  }
  else
  {
    s0 = left_available
      ? rshift_rnd_sf(s2, MB_BLOCK_SHIFT)           // upper edge
      : ref_pels[1];                              // top left corner, nothing to predict from
  }

  for (j = 0; j < MB_BLOCK_SIZE; j++)
  {
    for (i = 0; i < MB_BLOCK_SIZE; i++)
      pred_res[j][i] = (int) s0;    
  }
}

void JM_intra16x16_luma_plane(int **pred_res, int *ref_pels, int max_int_value)
{
  int i, j;
  // plane prediction
  int ih=0, iv=0;
  int ib, ic, iaa;
  int *t_pred = &ref_pels[25];
  int *u_pred = &ref_pels[8];
  int *b_pred = &ref_pels[23];

  for (i = 1; i < 8;++i)
  {
    ih += i*(*(u_pred + i) - *(u_pred - i));
    iv += i*(*t_pred++ - *b_pred--);
  }
  ih += 8 * (*(u_pred + 8) - ref_pels[0]);
  iv += 8 * (*t_pred++ - ref_pels[0]);

  ib = (5 * ih + 32) >> 6;
  ic = (5 * iv + 32) >> 6;

  iaa=16 * (ref_pels[16] + ref_pels[32]);

  for (j=0;j< MB_BLOCK_SIZE;++j)
  {
    for (i=0;i< MB_BLOCK_SIZE;++i)
    {
      pred_res[j][i]= (int) iClip1( max_int_value, rshift_rnd_sf((iaa + (i - 7) * ib + (j - 7) * ic), 5));// store plane prediction
    }
  }
}

void JM_chroma_vertical(int **pred_res, int *ref_pels, int cr_MB_x, int cr_MB_y)
{
  int j;
  for (j = 0; j < cr_MB_y; j++)
    memcpy(pred_res[j], &ref_pels[1], cr_MB_x * sizeof(int));
}

void JM_chroma_horizontal(int **pred_res, int *ref_pels, int cr_MB_x, int cr_MB_y)
{
  int i, j;
  int *prediction = &ref_pels[16];

  for (j = 0; j < cr_MB_y; j++)
  {  
    prediction++;
    for (i = 0; i < cr_MB_x; i++)
    {
      pred_res[j][i]  = *prediction;
    }    
  }
}

void JM_chroma_dc(int **pred_res, int *ref_pels, int left_available, int up_available)
{
  int i, j, s0 = 0, s1 = 0, s2 = 0;

  if (up_available)
  {
    for (i = 1; i < MB_BLOCK_SIZE + 1; ++i)
      s1 += ref_pels[i];
  }

  if (left_available)
  {
    for (i = 17; i < 33; ++i)
      s2 += ref_pels[i];    // sum vert pix
  }

  if (up_available)
  {
    s0 = left_available
      ? rshift_rnd_sf((s1 + s2),(MB_BLOCK_SHIFT + 1)) // no edge
      : rshift_rnd_sf(s1, MB_BLOCK_SHIFT);          // left edge
  }
  else
  {
    s0 = left_available
      ? rshift_rnd_sf(s2, MB_BLOCK_SHIFT)           // upper edge
      : ref_pels[1];                              // top left corner, nothing to predict from
  }

  for (j = 0; j < MB_BLOCK_SIZE; j++)
  {
    for (i = 0; i < MB_BLOCK_SIZE; i++)
      pred_res[j][i] = (int) s0;    
  }
}

void JM_chroma_plane(int **pred_res, int *ref_pels, int max_int_value)
{
  int i, j;
  // plane prediction
  int ih=0, iv=0;
  int ib, ic, iaa;
  int *t_pred = &ref_pels[25];
  int *u_pred = &ref_pels[8];
  int *b_pred = &ref_pels[23];

  for (i = 1; i < 8;++i)
  {
    ih += i*(*(u_pred + i) - *(u_pred - i));
    iv += i*(*t_pred++ - *b_pred--);
  }
  ih += 8 * (*(u_pred + 8) - ref_pels[0]);
  iv += 8 * (*t_pred++ - ref_pels[0]);

  ib = (5 * ih + 32) >> 6;
  ic = (5 * iv + 32) >> 6;

  iaa=16 * (ref_pels[16] + ref_pels[32]);

  for (j=0;j< MB_BLOCK_SIZE;++j)
  {
    for (i=0;i< MB_BLOCK_SIZE;++i)
    {
      pred_res[j][i]= (int) iClip1( max_int_value, rshift_rnd_sf((iaa + (i - 7) * ib + (j - 7) * ic), 5));// store plane prediction
    }
  }
}

*/
