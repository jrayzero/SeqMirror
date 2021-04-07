#ifndef MACROBLOCK
#define MACROBLOCK
#include "common.h"

struct video {
  int height;
  int width;
  int planes;
  int nframes;
  int chroma_height;
  int chroma_width;;
  int *y_data;
  int *u_data;
  int *v_data;
};

enum PLANE_TYPE {YPLANE, UPLANE, VPLANE};

struct macroblock {
  enum PLANE_TYPE plane_type;
  // location info
  int frame_idx;
  int origin_row;
  int origin_col;
  // holds prediction + eventually data to-be-entropy-coded
  int *pred_16x16_vertical;
  int *pred_16x16_horizontal;
  int *pred_16x16_DC;
  int *pred_16x16_planar;
  // ptr to the video data  
  struct video *vid;  
};

#endif
