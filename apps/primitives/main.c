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

  const char *input_file = "foreman_part_qcif_x1000.yuv";
  int niters = 10;
  int nframes = 100;
  int height = 144;
  int width = 176;
  struct video *vid = (struct video*)malloc(sizeof(struct video));
  ingest(input_file, height, width, nframes, vid);  
  unsigned long total_time = 0;
  for (int n = 0; n < niters; n++) {
    struct timespec tick = ctick();
    // get the data
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
	  pred_luma_16x16(&mb);
	}
      }
    }
    total_time += ctock(tick, 1);
    /*    for (int f = 0; f < vid->nframes; f++) {
      // Y plane
      for (int i = 0; i < vid->height; i++) {
	for (int j = 0; j < vid->width; j++) {
	  fprintf(stderr, "%d ", vid->y_data[f*vid->height*vid->width + i*vid->width + j]);
	}
	fprintf(stderr, "\n");
      }
      fprintf(stderr, "\n");
      }*/
  }
  fprintf(stderr, " %f\n", total_time/1e9/niters);
  free(vid->y_data);
  free(vid->u_data);
  free(vid->v_data);  
}
