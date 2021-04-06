#include "common.h"
#include "quant.h"

int quant_ac4x4_normal(int **input_block, int **output_block, int **pos_scan, int block_x_pos) {
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
}
 
// Takes in a 4x4 block of DC coefficients, which represents one MB worth of DCs used with a 4x4 transform
int quant_dc4x4_normal(int **input_block, int **output_block, int **pos_scan)
{
  int i,j, coeff_ctr;

  int pmin;
  int scaled_coeff;

  int   level;
  int   nonzero = FALSE;  
  int   qp_per = p_Quant->qp_per_matrix[qp];
  int   q_bits = Q_BITS + qp_per + 1;

  const byte *p_scan = &pos_scan[0][0];

  // Quantization
  for (coeff_ctr = 0; coeff_ctr < 16; ++coeff_ctr)
  {
    i = *p_scan++;  // horizontal position
    j = *p_scan++;  // vertical position

    pmin = input_block[j][i];

    if (pmin != 0)
    {    
      scaled_coeff = iabs (pmin) * q_params_4x4->ScaleComp;
      level = (scaled_coeff + (q_params_4x4->OffsetComp << 1) ) >> q_bits;

      if (level != 0)
      {
        level   = isignab(level, pmin);
        output_block[j][i]     = level;
        nonzero = TRUE;
      }
      else
      {
        output_block[j][i] = 0;
      }
    }
  }

  return nonzero;
}
