#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "intra.h"
#include "transform.h"

bool intra16x16_luma_vertical(struct macroblock *mb) {
  if (mb->origin_row > 0) {
    int *ref_pels = &mb->vid->y_data[(mb->frame_idx * mb->vid->width * mb->vid->height) +
				     (mb->origin_row-1)*mb->vid->width + mb->origin_col];
    for (int j = 0; j < MB_BLOCK_SIZE; j++)
      memcpy(&mb->pred_16x16_vertical[j * MB_BLOCK_SIZE], ref_pels, MB_BLOCK_SIZE * sizeof(int));
    return true;
  }
  return false;
}

bool intra16x16_luma_horizontal(struct macroblock *mb) {
  if (mb->origin_col > 0) {
    for (int j = 0; j < MB_BLOCK_SIZE; j++) {
      int ref_pel = mb->vid->y_data[(mb->frame_idx * mb->vid->width * mb->vid->height) +
				    (j+mb->origin_row) * mb->vid->width + mb->origin_col - 1];
      for (int i = 0; i < MB_BLOCK_SIZE; i++) {
	mb->pred_16x16_horizontal[j * MB_BLOCK_SIZE + i] = ref_pel;
      }
    }
    return true;
  }
  return false;
}

void intra16x16_luma_DC(struct macroblock *mb) {
  int sum = 0;
  if (mb->origin_row > 0 && mb->origin_col > 0) {
    int *ref_row = &mb->vid->y_data[(mb->frame_idx * mb->vid->width * mb->vid->height) +
				    (mb->origin_row-1)*mb->vid->width + mb->origin_col];
    sum = 16;
    for (int j = 0; j < MB_BLOCK_SIZE; j++) {
      int r = ref_row[j];
      int c = mb->vid->y_data[(mb->frame_idx * mb->vid->width * mb->vid->height) +
			      (j+mb->origin_row) * mb->vid->width + mb->origin_col - 1];
      sum += r + c;
    }
    sum >>= 5;
  } else if (mb->origin_col > 0) {
    sum = 8;
    for (int j = 0; j < MB_BLOCK_SIZE; j++) {
      int ref_pel = mb->vid->y_data[(mb->frame_idx * mb->vid->width * mb->vid->height) +
				    (j+ mb->origin_row) * mb->vid->width + mb->origin_col - 1];
      sum += ref_pel;
    }
    sum >>= 4;
  } else if (mb->origin_row > 0) {
    sum = 8;
    int *ref_row = &mb->vid->y_data[(mb->frame_idx * mb->vid->width * mb->vid->height) +
				    (mb->origin_row-1)*mb->vid->width + mb->origin_col];
    for (int j = 0; j < MB_BLOCK_SIZE; j++) {
      sum += *ref_row++;
    }
    sum >>= 4;
  } else {
    sum = 1 << (bit_depth_y - 1);
  }
  
  for (int j = 0; j < MB_BLOCK_SIZE; j++) {
    for (int i = 0; i < MB_BLOCK_SIZE; i++) {
      mb->pred_16x16_DC[j * MB_BLOCK_SIZE + i] = sum;
    }
  }  
}

bool intra16x16_luma_planar(struct macroblock *mb) {
  if (mb->origin_row > 0 && mb->origin_col > 0) {
    int *ref_row = &mb->vid->y_data[(mb->frame_idx * mb->vid->width * mb->vid->height) +
				    (mb->origin_row-1)*mb->vid->width + mb->origin_col];
    int *pH1 = ref_row + 8;
    int *pH2 = ref_row;
    int H = 0;
    int r = 0;
    for (int j = 0; j < 8; j++) {
      H += (r+1)*(*pH1++-*pH2++);
      r++;
    }
    int V = 0;
    r = 0;
    for (int j = 0; j < 8; j++) {
      int pV1 = mb->vid->y_data[(mb->frame_idx * mb->vid->width * mb->vid->height) +
				(j+mb->origin_row+8) * mb->vid->width + mb->origin_col - 1];
      int pV2 = mb->vid->y_data[(mb->frame_idx * mb->vid->width * mb->vid->height) +
				(j+mb->origin_row) * mb->vid->width + mb->origin_col - 1];
      V += (r+1)*(pV1-pV2);
      r++;
    }
    int a = 16 * (*(ref_row+15) + mb->vid->y_data[(mb->frame_idx * mb->vid->width * mb->vid->height) +
						  (mb->origin_row+15) * mb->vid->width + mb->origin_col - 1]);
    int b = (5 * H + 32) >> 6;
    int c = (5 * V + 32) >> 6;
    for (int j = 0; j < MB_BLOCK_SIZE; j++) {
      for (int i = 0; i < MB_BLOCK_SIZE; i++) {
	int clip_val = (a + b * (i - 7) + c * (j - 7) + 16) >> 5;
	mb->pred_16x16_planar[j * MB_BLOCK_SIZE + i] = clip1y(clip_val);
      }
    }
    return true;
  }
  return false;
}

/*

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
