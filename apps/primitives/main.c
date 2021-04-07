#include "macroblock.h"
#include "intra.h"
#include "ticky.h"
#include "read_yuv.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

int main() {
  struct timespec tick = ctick();
  const char *input_file = "foreman_part_qcif_x100.yuv";
  int nframes = 100;
  int height = 144;
  int width = 176;
  struct video *vid = (struct video*)malloc(sizeof(struct video));
  int niters = 10;
  for (int n = 0; n < niters; n++) {
    // get the data
    ingest(input_file, height, width, nframes, vid);
#ifdef DEBUG
    int mbidx = 0;
    FILE *intra_pred_fd = fopen("pred16x16.c.out", "w");
#endif
    // go through macroblocks
    for (int f = 0; f < vid->nframes; f++) {
      // Y plane
      for (int i = 0; i < vid->height; i+=16) {
	for (int j = 0; j < vid->width; j+=16) {
	  struct macroblock mb;
	  mb.plane_type = YPLANE;
	  mb.vid = vid;
	  mb.frame_idx = f;
	  mb.origin_row = i;
	  mb.origin_col = j;
	  mb.pred_16x16_vertical = (int*)malloc(sizeof(int)*16*16);
	  mb.pred_16x16_horizontal = (int*)malloc(sizeof(int)*16*16);
	  mb.pred_16x16_DC = (int*)malloc(sizeof(int)*16*16);
	  mb.pred_16x16_planar = (int*)malloc(sizeof(int)*16*16);
	  // intraprediction
	  int costs[4] = {INT_MAX, INT_MAX, INT_MAX, INT_MAX};
	  bool did_vertical = intra16x16_luma_vertical(&mb);
	  bool did_horizontal = intra16x16_luma_horizontal(&mb);
	  intra16x16_luma_DC(&mb);
	  bool did_planar = intra16x16_luma_planar(&mb);
	  DO(fprintf(stderr,"mb: %d => vert %d, horiz %d, dc 1, planar %d\n", mbidx, did_vertical, did_horizontal, did_planar));
	  // compute a fake cost
	  if (did_vertical) {
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
	  }
	  free(mb.pred_16x16_vertical);
	  free(mb.pred_16x16_horizontal);
	  free(mb.pred_16x16_DC);
	  free(mb.pred_16x16_planar);
	}
      }
    }
    free(vid->y_data);
    free(vid->u_data);
    free(vid->v_data);
  }
  ctock(tick, niters);
    
}
