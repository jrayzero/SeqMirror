#include "macroblock.h"
#include <stdlib.h>

int sad(struct macroblock *mb, int *pred) {
  int cost = 0;
  for (int j = 0; j < MB_BLOCK_SIZE; j++) {
    for (int i = 0; i < MB_BLOCK_SIZE; i++) {
      int idx = j * MB_BLOCK_SIZE + i;
      cost += abs(mb->vid->y_data[(mb->frame_idx * mb->vid->width * mb->vid->height) + (mb->origin_row + j) * mb->vid->width + mb->origin_col + i] - pred[idx]);
    }
  }  
  return cost;
}
