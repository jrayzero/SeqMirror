#ifndef CONTROL
#define CONTROL

#include <stdio.h>
#include "macroblock.h"

void pred_luma_16x16(struct macroblock *mb);
void xform_quant_luma_16x16(struct macroblock *mb, int *pred, int*, int*);

#endif
