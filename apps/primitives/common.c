#include "common.h"

int clip3(int x, int y, int z) {
  if (z < x)
    return x;
  else if (z > y)
    return y;
  else
    return z;
}

int clip1y(int x) {
  return clip3(0, (1<<bit_depth_y)-1,x);
}
