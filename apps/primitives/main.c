#include "control.h"
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
  int niters = 1;
  int nframes = 3;
  int height = 144;
  int width = 176;
  struct video *vid = (struct video*)malloc(sizeof(struct video));
#ifdef DEBUG
    int mbidx = 0;
    FILE *debug = fopen("debug.c.out", "w");
#else
    FILE *debug = NULL;
#endif
  
  for (int n = 0; n < niters; n++) {
    // get the data
    ingest(input_file, height, width, nframes, vid);
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
	  pred_luma_16x16(&mb, debug);
	}
      }
    }
    free(vid->y_data);
    free(vid->u_data);
    free(vid->v_data);
  }
  ctock(tick, niters);
#ifdef DEBUG
  fflush(debug);
  fclose(debug);
#endif
    
}
