#include "control.h"
#include <stdlib.h>
#include "cost.h"
#include "intra.h"
#include "transform.h"
#include <limits.h>
#include "quant.h"

#define rshift_rnd_sf(x,a) (((x) + (1 << ((a)-1) )) >> (a))
// do prediction, call the transform loop, get the costs
void pred_luma_16x16(struct macroblock *mb) {
  mb->pred_16x16_vertical = (int*)malloc(sizeof(int)*16*16);
  mb->pred_16x16_horizontal = (int*)malloc(sizeof(int)*16*16);
  mb->pred_16x16_DC = (int*)malloc(sizeof(int)*16*16);
  mb->pred_16x16_planar = (int*)malloc(sizeof(int)*16*16);
  // prediction
  bool did_vertical = intra16x16_luma_vertical(mb);
  bool did_horizontal = intra16x16_luma_horizontal(mb);
  intra16x16_luma_DC(mb);
  bool did_planar = intra16x16_luma_planar(mb);
  
  int min_cost = INT_MAX;
  int *best = NULL;
  int midx = 0;
  if (did_vertical) {
    int *transformed = (int*)malloc(sizeof(int) * 16 * 16);
    int *reconstruction = (int*)malloc(sizeof(int) * 16 * 16);
    xform_quant_luma_16x16(mb, mb->pred_16x16_vertical, transformed, reconstruction);
    int cost = sad(mb, reconstruction);
    if (cost < min_cost) {
      min_cost = cost;
      best = reconstruction;
    }
  }
  if (did_horizontal) {
    int *transformed = (int*)malloc(sizeof(int) * 16 * 16);
    int *reconstruction = (int*)malloc(sizeof(int) * 16 * 16);
    xform_quant_luma_16x16(mb, mb->pred_16x16_horizontal, transformed, reconstruction);
    int cost = sad(mb, reconstruction);
    if (cost < min_cost) {
      min_cost = cost;
      best = reconstruction;
      midx = 1;
    }
  }
  int *transformed = (int*)malloc(sizeof(int) * 16 * 16);
  int *reconstruction = (int*)malloc(sizeof(int) * 16 * 16);  
  xform_quant_luma_16x16(mb, mb->pred_16x16_DC, transformed, reconstruction);
  int cost = sad(mb, reconstruction);
  if (cost < min_cost) {
    min_cost = cost;
    best = reconstruction;
    midx = 2;
  }
  if (did_planar) {
    int *transformed = (int*)malloc(sizeof(int) * 16 * 16);
    int *reconstruction = (int*)malloc(sizeof(int) * 16 * 16);    
    xform_quant_luma_16x16(mb, mb->pred_16x16_planar, transformed, reconstruction);
    int cost = sad(mb, reconstruction);
    if (cost < min_cost) {
      min_cost = cost;
      best = reconstruction;
      midx = 3;
    }
  }
  
  for (int j = 0; j < MB_BLOCK_SIZE; j++) {
    for (int i = 0; i < MB_BLOCK_SIZE; i++) {
      int idx = j * MB_BLOCK_SIZE + i;
      mb->vid->y_data[(mb->frame_idx * mb->vid->width * mb->vid->height) + (mb->origin_row + j) * mb->vid->width + mb->origin_col + i] = best[idx];
    }
  }
  
  free(mb->pred_16x16_vertical);
  free(mb->pred_16x16_horizontal);
  free(mb->pred_16x16_DC);
  free(mb->pred_16x16_planar);
}

void xform_quant_luma_16x16(struct macroblock *mb, int *pred, int *transformed, int *reconstruction) {
  // compute the residual
  int *residual = (int*)malloc(sizeof(int) * 16 * 16);
  for (int j = 0; j < MB_BLOCK_SIZE; j++) {
    for (int i = 0; i < MB_BLOCK_SIZE; i++) {
      int idx = j * MB_BLOCK_SIZE + i;
      residual[idx] = mb->vid->y_data[(mb->frame_idx * mb->vid->width * mb->vid->height) + (mb->origin_row + j) * mb->vid->width + mb->origin_col + i] - pred[idx];
    }
  }
  // compute 4x4 transforms across the residual

  for (int i = 0; i < MB_BLOCK_SIZE; i+=4) {
    for (int j = 0; j < MB_BLOCK_SIZE; j+=4) {
      forward4x4(residual, transformed, i, j);
    }
  }
  // gather the DCs and transform them
  int *transformed_DCs = (int*)malloc(sizeof(int) * 16);
  hadamard4x4_DCs(transformed, transformed_DCs);
  // quantize the DCs
  bool nonzero = quant_dc4x4_normal(transformed_DCs, QP);
  // inverse the DCs and store in the reconstruction
  if (nonzero) {
    ihadamard4x4_DCs(transformed_DCs, reconstruction);
    inverse_quant_DCs(reconstruction, QP);
  } else {
    for (int i = 0; i < 16; i+=4) {
      for (int j = 0; j < 16; j+=4) {
	reconstruction[i * 16 + j] = 0;
      }
    }
  }
  for (int i = 0; i < 16; i+=4) {
    for (int j = 0; j < 16; j+=4) {
      transformed[i * 16 + j] = reconstruction[i * 16 + j];
    }
  }
  
  for (int i = 0; i < 16; i+=4) {
    for (int j = 0; j < 16; j+=4) {
      quant_ac4x4_normal(transformed, QP, i, j);
      inverse4x4(transformed, reconstruction, i, j);
    }
  }

  // now do the final reconstruction
  int idx = 0;
  for (int i = 0; i < 16; i+=4) {
    for (int j = 0; j < 16; j+=4) {
      transformed[i * 16 + j] = transformed_DCs[idx];
      idx++;
    }
  }  
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 16; j++) {
      int val = rshift_rnd_sf(reconstruction[i*16+j], 6) + pred[i*16+j];
      val = 0 > val ? 0 : val;
      val = 255 < val ? 255 : val;
      reconstruction[i*16+j] = val;
    }
  }  
  
}
