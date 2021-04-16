/*!
*************************************************************************************
 * \file intra16x16.c
 *
 * \brief
 *    Intra 16x16 mode functions
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Alexis Michael Tourapis         <alexismt@ieee.org>
 *************************************************************************************
 */

#include "contributors.h"

#include "global.h"
#include "image.h"
#include "mb_access.h"
#include "transform.h"
#include "memalloc.h"

/*!
 ************************************************************************
 * \brief
 *    Vertical 16x16 Prediction
 ************************************************************************
 */
static inline void get_i16x16_vertical(imgpel **cur_pred, imgpel *PredPel)
{
  int j;
  for (j = 0; j < MB_BLOCK_SIZE; j++)
    memcpy(cur_pred[j], &PredPel[1], MB_BLOCK_SIZE * sizeof(imgpel));
}


/*!
 ************************************************************************
 * \brief
 *    Horizontal 16x16 Prediction
 ************************************************************************
 */
static inline void get_i16x16_horizontal(imgpel **cur_pred, imgpel *PredPel)
{
  int i, j;
  imgpel *prediction = &PredPel[16];

  for (j = 0; j < MB_BLOCK_SIZE; j++)
  {  
    prediction++;
    for (i = 0; i < MB_BLOCK_SIZE; i++)
    {
      cur_pred[j][i]  = *prediction;
    }    
  }
}

/*!
 ************************************************************************
 * \brief
 *    DC 16x16 Prediction
 ************************************************************************
 */
static inline void get_i16x16_dc(imgpel **cur_pred, imgpel *PredPel, int left_available, int up_available)
{
  int i, j, s0 = 0, s1 = 0, s2 = 0;

  if (up_available)
  {
    for (i = 1; i < MB_BLOCK_SIZE + 1; ++i)
      s1 += PredPel[i];
  }

  if (left_available)
  {
    for (i = 17; i < 33; ++i)
      s2 += PredPel[i];    // sum vert pix
  }

  if (up_available)
  {
    s0 = left_available
      ? rshift_rnd_sf((s1 + s2),(MB_BLOCK_SHIFT + 1)) // no edge
      : rshift_rnd_sf(s1, MB_BLOCK_SHIFT);          // left edge
  }
  else
  {
    s0 = left_available
      ? rshift_rnd_sf(s2, MB_BLOCK_SHIFT)           // upper edge
      : PredPel[1];                              // top left corner, nothing to predict from
  }

  for (j = 0; j < MB_BLOCK_SIZE; j++)
  {
    for (i = 0; i < MB_BLOCK_SIZE; i++)
      cur_pred[j][i] = (imgpel) s0;    
  }
}

/*!
 ************************************************************************
 * \brief
 *    Plane 16x16 Prediction
 ************************************************************************
 */
static inline void get_i16x16_plane(imgpel **cur_pred, imgpel *PredPel, int max_imgpel_value)
{
  int i, j;
  // plane prediction
  int ih=0, iv=0;
  int ib, ic, iaa;
  imgpel *t_pred = &PredPel[25];
  imgpel *u_pred = &PredPel[8];
  imgpel *b_pred = &PredPel[23];

  for (i = 1; i < 8;++i)
  {
    ih += i*(*(u_pred + i) - *(u_pred - i));
    iv += i*(*t_pred++ - *b_pred--);
  }
  ih += 8 * (*(u_pred + 8) - PredPel[0]);
  iv += 8 * (*t_pred++ - PredPel[0]);

  ib = (5 * ih + 32) >> 6;
  ic = (5 * iv + 32) >> 6;

  iaa=16 * (PredPel[16] + PredPel[32]);

  for (j=0;j< MB_BLOCK_SIZE;++j)
  {
    for (i=0;i< MB_BLOCK_SIZE;++i)
    {
      cur_pred[j][i]= (imgpel) iClip1( max_imgpel_value, rshift_rnd_sf((iaa + (i - 7) * ib + (j - 7) * ic), 5));// store plane prediction
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Set intra 16x16 prediction samples
 *
 *  \par Input:
 *
 *  \par Output:
 *      none
 ************************************************************************
 */
void set_intrapred_16x16(Macroblock *currMB, ColorPlane pl, int *left_available, int *up_available, int *all_available)
{
  VideoParameters *p_Vid = currMB->p_Vid;
  InputParameters *p_Inp = currMB->p_Inp;

  int i;
  imgpel  *PredPel = currMB->intra16x16_pred[pl];  // array of predictor pels
  imgpel **img_enc = (currMB->p_Slice->P444_joined)? p_Vid->enc_picture->p_img[pl]: p_Vid->enc_picture->p_curr_img;

  PixelPos pix_b;
  PixelPos pix_a;
  PixelPos pix_d;

  int *mb_size = p_Vid->mb_size[IS_LUMA];

  p_Vid->getNeighbour(currMB, -1,  -1, mb_size, &pix_d);
  p_Vid->getNeighbour(currMB, -1,    0, mb_size, &pix_a);
  p_Vid->getNeighbour(currMB,    0,   -1, mb_size, &pix_b);

  if (p_Inp->UseConstrainedIntraPred == 0)
  {
    *up_available   = pix_b.available;
    *left_available = pix_a.available;
    *all_available  = pix_d.available;
  }
  else
  {
    *up_available   = pix_b.available ? p_Vid->intra_block[pix_b.mb_addr] : 0;
    *left_available = pix_a.available ? p_Vid->intra_block[pix_a.mb_addr] : 0;
    *all_available  = pix_d.available ? p_Vid->intra_block[pix_d.mb_addr] : 0;
  }

  // form predictor pels
  if (*up_available)
  {
    memcpy(&PredPel[1], &img_enc[pix_b.pos_y][pix_b.pos_x], MB_BLOCK_SIZE * sizeof(imgpel));
  }
  else
  {
    for (i = 1; i < 17; ++i)
      PredPel[i] = (imgpel) p_Vid->dc_pred_value;
  }

  if (*left_available)
  {
    int pos_y = pix_a.pos_y;
    int pos_x = pix_a.pos_x;

    for (i = 1; i < MB_BLOCK_SIZE + 1; ++i)
      PredPel[i + 16] = img_enc[pos_y++][pos_x];
  }
  else
  {
    for (i = 17; i < 33; ++i)
      PredPel[i] = (imgpel) p_Vid->dc_pred_value;
  }

  if (*all_available)
  {
    PredPel[0] = img_enc[pix_d.pos_y][pix_d.pos_x];
  }
  else
  {
    PredPel[0] = p_Vid->dc_pred_value;
  }
}
