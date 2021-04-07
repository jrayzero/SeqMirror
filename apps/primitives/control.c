#include "control.h"
#include <stdlib.h>
#include "intra.h"
#include "transform.h"
#include <limits.h>

// do prediction, call the transform loop, get the costs
void pred_luma_16x16(struct macroblock *mb, FILE *fd) {
  mb->pred_16x16_vertical = (int*)malloc(sizeof(int)*16*16);
  mb->pred_16x16_horizontal = (int*)malloc(sizeof(int)*16*16);
  mb->pred_16x16_DC = (int*)malloc(sizeof(int)*16*16);
  mb->pred_16x16_planar = (int*)malloc(sizeof(int)*16*16);
  int costs[4] = {INT_MAX, INT_MAX, INT_MAX, INT_MAX};
  // prediction
  bool did_vertical = intra16x16_luma_vertical(mb);
  bool did_horizontal = intra16x16_luma_horizontal(mb);
  intra16x16_luma_DC(mb);
  bool did_planar = intra16x16_luma_planar(mb);
  if (did_vertical) {
    xform_quant_luma_16x16(mb, mb->pred_16x16_vertical, fd);
  }
  if (did_horizontal) {
    xform_quant_luma_16x16(mb, mb->pred_16x16_horizontal, fd);
  }
  xform_quant_luma_16x16(mb, mb->pred_16x16_DC, fd);
  if (did_planar) {
    xform_quant_luma_16x16(mb, mb->pred_16x16_planar, fd);
  }  
  
  // compute a fake cost
  /*  if (did_vertical) {
    int cost = 0;
    for (int j = 0; j < MB_BLOCK_SIZE; j++) {
      for (int i = 0; i < MB_BLOCK_SIZE; i++) {
	cost += mb.pred_16x16_vertical[j*MB_BLOCK_SIZE+i];
      }
    }
    costs[0] = cost;
  }
  if (did_horizontal) {
    int cost = 0;
    for (int j = 0; j < MB_BLOCK_SIZE; j++) {
      for (int i = 0; i < MB_BLOCK_SIZE; i++) {
	cost += mb.pred_16x16_horizontal[j*MB_BLOCK_SIZE+i];
      }
    }
    costs[1] = cost;
  }
  int cost = 0;
  for (int j = 0; j < MB_BLOCK_SIZE; j++) {
    for (int i = 0; i < MB_BLOCK_SIZE; i++) {
      cost += mb.pred_16x16_DC[j*MB_BLOCK_SIZE+i];
    }
  }
  costs[2] = cost;
  if (did_planar) {
    int cost = 0;
    for (int j = 0; j < MB_BLOCK_SIZE; j++) {
      for (int i = 0; i < MB_BLOCK_SIZE; i++) {
	cost += mb.pred_16x16_planar[j*MB_BLOCK_SIZE+i];
      }
    }
    costs[3] = cost;
    }*/
  //  free(mb.pred_16x16_vertical);
  //  free(mb.pred_16x16_horizontal);
  //  free(mb.pred_16x16_DC);
  //  free(mb.pred_16x16_planar);
}

void xform_quant_luma_16x16(struct macroblock *mb, int *pred, FILE *fd) {
  // compute the residual
  int *residual = (int*)malloc(sizeof(int) * 16 * 16);
  for (int j = 0; j < MB_BLOCK_SIZE; j++) {
    for (int i = 0; i < MB_BLOCK_SIZE; i++) {
      int idx = j * MB_BLOCK_SIZE + i;
      residual[idx] = mb->vid->y_data[(mb->frame_idx * mb->vid->width * mb->vid->height) + (mb->origin_row + j) * mb->vid->width + mb->origin_col + i] - pred[idx];
    }
  }
  // compute 4x4 transforms across the residual
  int *transformed = (int*)malloc(sizeof(int) * 16 * 16);
  for (int i = 0; i < MB_BLOCK_SIZE; i+=4) {
    for (int j = 0; j < MB_BLOCK_SIZE; j+=4) {
      forward4x4(residual, transformed, i, j);
    }
  }
  // gather the DCs and transform them
  int *transformed_DCs = (int*)malloc(sizeof(int) * 16);
  hadamard4x4_DCs(transformed, transformed_DCs);
  
  
  for (int i = 0; i < 4; i+=1) {
    for (int j = 0; j < 4; j+=1) {
      fprintf(fd, "%d ", transformed_DCs[i*4+j]);
    }
    fprintf(fd, "\n");
  }
  fprintf(fd, "\n");
  // create a temporary reconstruction
  //  int *reconstruction = (int*)malloc(sizeof(int) * 16 * 16);
  
  
}
