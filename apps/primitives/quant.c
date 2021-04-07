#include "common.h"
#include "quant.h"
#include <stdbool.h>
#include <stdlib.h>

/*int quant_ac4x4_normal(int **input_block, int **output_block, int **pos_scan, int block_x_pos) {
  int i,j, coeff_ctr;

  int pmin;
  int scaled_coeff;
  bool nonzero = FALSE;
  int   level = 0;
  int   qp_per = p_Quant->qp_per_matrix[qp];
  int   q_bits = Q_BITS + qp_per;
  const int *p_scan = &pos_scan[1][0];

  // Quantization
  for (coeff_ctr = 1; coeff_ctr < 16; ++coeff_ctr)
  {
    i = *p_scan++;  // horizontal position
    j = *p_scan++;  // vertical position

    pmin = input_block[j][block_x + i];
    if (pmin != 0)
    {
      q_params = &q_params_4x4[j][i];
      scaled_coeff = iabs (pmin) * q_params->ScaleComp;
      level = (scaled_coeff + q_params->OffsetComp) >> q_bits;

      if (level != 0)
      {
        level  = isignab(level, pmin);
        output_block[j][block_x+i] = rshift_rnd_sf(((level * q_params->InvScaleComp) << qp_per), 4);
        nonzero = TRUE;
      }
      else
      {
	output_block[j][block_x+i] = 0;
      }
    }
  }

  return nonzero;
  }*/


int qp_per_matrix[51] = {0,0,0,0,0,0,1,1,1,1,1,1,2,2,2,2,2,2,3,3,3,3,3,3,4,4,4,4,4,4,5,5,5,5,5,5,6,6,6,6,6,6,7,7,7,7,7,7,8,8,8};
int scale_comp_DC[16] = {8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192};
// Takes in a 4x4 block of DC coefficients, which represents one MB worth of DCs used with a 4x4 transform
bool quant_dc4x4_normal(int *block, int qp) {
  int   nonzero = false;  
  int   qp_per = qp_per_matrix[qp];
  int   q_bits = Q_BITS + qp_per + 1;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      int coeff = block[i * 4 + j];
      if (coeff != 0) {	
	int scaled = abs(coeff) * scale_comp_DC[i * 4 + j];
	int level = (scaled + (offset_comp << 1)) >> q_bits;
	if (level != 0) {
	  level = coeff < 0 ? abs(level) * -1 : abs(level);
	  block[i * 4 + j] = level;
	  nonzero = true;
	} else {
	  block[i * 4 + j] = 0;
	}
      }
    } 
  }
  return nonzero;
}
