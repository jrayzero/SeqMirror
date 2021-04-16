/*!
************************************************************************
* \brief
*    UMHEXIntegerPelBlockMotionSearch: fast pixel block motion search
*    this algorithm is called UMHexagonS(see JVT-D016),which includes
*    four steps with different kinds of search patterns
* \par Input:
* imgpel*   orig_pic,     // <--  original picture
* int       ref,          // <--  reference frame (0... or -1 (backward))
* int       pic_pix_x,    // <--  absolute x-coordinate of regarded AxB block
* int       pic_pix_y,    // <--  absolute y-coordinate of regarded AxB block
* int       blocktype,    // <--  block type (1-16x16 ... 7-4x4)
* int       pred_mv[2],   // <--  motion vector predictor (x|y) in sub-pel units
* MotionVector   *mv,        //  --> motion vector (x|y) - in sub-pel units
* int       search_range, // <--  1-d search range in sub-pel units
* int       min_mcost,    // <--  minimum motion cost (cost for center or huge value)
* int       lambda_factor // <--  lagrangian parameter for determining motion cost
* \par
* Two macro definitions defined in this program:
* 1. EARLY_TERMINATION: early termination algrithm, refer to JVT-D016.doc
* 2. SEARCH_ONE_PIXEL: search one pixel in search range
* \author
*   Main contributors: (see contributors.h for copyright, address and affiliation details)
*   - Zhibo Chen         <chenzhibo@tsinghua.org.cn>
*   - JianFeng Xu        <fenax@video.mdc.tsinghua.edu.cn>
*   - Xiaozhong Xu       <xxz@video.mdc.tsinghua.edu.cn>
* \date   :
*   2006.1
************************************************************************
*/
distblk                                     //  ==> minimum motion cost after search
UMHEXIntegerPelBlockMotionSearch  (Macroblock *currMB,     // <--  current Macroblock
                                   MotionVector *pred_mv,    // < <--  motion vector predictor (x|y) in sub-pel units
                                   MEBlock *mv_block,
                                   distblk     min_mcost,     // < <--  minimum motion cost (cost for center or huge value)
                                   int       lambda_factor  // < <--  lagrangian parameter for determining motion cost
                                   )
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  InputParameters *p_Inp = currMB->p_Inp;
  UMHexStruct *p_UMHex = p_Vid->p_UMHex;

  int   blocktype     = mv_block->blocktype;
  short blocksize_x   = mv_block->blocksize_x;  // horizontal block size
  short blocksize_y   = mv_block->blocksize_y;  // vertical block size
  short pic_pix_x2    = mv_block->pos_x2;
  short block_x       = mv_block->block_x;
  short block_y       = mv_block->block_y;

  int   list = mv_block->list;
  int   cur_list = list + currMB->list_offset;
  short ref = mv_block->ref_idx;
  StorablePicture *ref_picture = currSlice->listX[cur_list][ref];

  MotionVector *mv = &mv_block->mv[list];
  MotionVector iMinNow, cand, center, pred, best = {0, 0};

  int   search_step;
  int   pos;
  distblk mcost;
  distblk   *SAD_prediction = p_UMHex->fastme_best_cost[blocktype-1];//multi ref SAD prediction
  int   i, j, m;
  float betaFourth_1,betaFourth_2;
  int  temp_Big_Hexagon_X[16];//  temp for Big_Hexagon_X;
  int  temp_Big_Hexagon_Y[16];//  temp for Big_Hexagon_Y;
  distblk ET_Thred = p_UMHex->Median_Pred_Thd_MB[blocktype];//ET threshold in use
  int  search_range = mv_block->searchRange.max_x >> 2;

  short pic_pix_x = mv_block->pos_x_padded;
  short pic_pix_y = mv_block->pos_y_padded;
  pred.mv_x   = pic_pix_x + pred_mv->mv_x;       // predicted position x (in sub-pel units)
  pred.mv_y   = pic_pix_y + pred_mv->mv_y;       // predicted position y (in sub-pel units)
  center.mv_x = pic_pix_x + mv->mv_x;            // center position x (in sub-pel units)
  center.mv_y = pic_pix_y + mv->mv_y;            // center position y (in sub-pel units)



  //////allocate memory for search state//////////////////////////
  memset(p_UMHex->McostState[0],0,(2*p_Inp->search_range[p_Vid->view_id]+1)*(2*p_Inp->search_range[p_Vid->view_id]+1));


  //check the center median predictor

  cand = center;
  mcost = mv_cost (p_Vid, lambda_factor, &cand, &pred);

  mcost += mv_block->computePredFPel(ref_picture, mv_block, min_mcost - mcost, &cand);

  p_UMHex->McostState[search_range][search_range] = 1;
  if (mcost < min_mcost)
  {
    min_mcost = mcost;
    best = cand;
  }

  iMinNow = best;

  for (m = 0; m < 4; m++)
  {
    cand.mv_x = iMinNow.mv_x + Diamond[m].mv_x;
    cand.mv_y = iMinNow.mv_y + Diamond[m].mv_y;
    SEARCH_ONE_PIXEL
  }

  if(center.mv_x != pic_pix_x || center.mv_y != pic_pix_y)
  {
    cand.mv_x = pic_pix_x ;
    cand.mv_y = pic_pix_y ;
    SEARCH_ONE_PIXEL
      iMinNow = best;  
    for (m = 0; m < 4; m++)
    {
      cand.mv_x = iMinNow.mv_x + Diamond[m].mv_x;
      cand.mv_y = iMinNow.mv_y + Diamond[m].mv_y;
      SEARCH_ONE_PIXEL
    }
  }
  /***********************************init process*************************/
  //for multi ref
  if(ref>0 && currSlice->structure == FRAME  && min_mcost > ET_Thred && SAD_prediction[pic_pix_x2] < p_UMHex->Multi_Ref_Thd_MB[blocktype])
    goto terminate_step;

  //ET_Thd1: early termination for low motion case
  if( min_mcost < ET_Thred)
  {
    goto terminate_step;
  }
  else // hybrid search for main search loop
  {
    /****************************(MV and SAD prediction)********************************/
    UMHEX_setup(currMB, ref, mv_block->list, block_y, block_x, blocktype, currSlice->all_mv );
    ET_Thred = p_UMHex->Big_Hexagon_Thd_MB[blocktype];  // ET_Thd2: early termination Threshold for strong motion

    // Threshold defined for EARLY_TERMINATION
    if (p_UMHex->pred_SAD == 0)
    {
      betaFourth_1=0;
      betaFourth_2=0;
    }
    else
    {
      betaFourth_1 = p_UMHex->Bsize[blocktype]/((float)p_UMHex->pred_SAD * p_UMHex->pred_SAD)-p_UMHex->AlphaFourth_1[blocktype];
      betaFourth_2 = p_UMHex->Bsize[blocktype]/((float)p_UMHex->pred_SAD * p_UMHex->pred_SAD)-p_UMHex->AlphaFourth_2[blocktype];

    }
    /*********************************************end of init ***********************************************/
  }
  // first_step: initial start point prediction

  if(blocktype>1)
  {
    cand.mv_x = (short) (pic_pix_x + (p_UMHex->pred_MV_uplayer[0] / 4) * 4);
    cand.mv_y = (short) (pic_pix_y + (p_UMHex->pred_MV_uplayer[1] / 4) * 4);
    SEARCH_ONE_PIXEL
  }


  //prediction using mV of last ref moiton vector
  if(p_UMHex->pred_MV_ref_flag == 1)      //Notes: for interlace case, ref==1 should be added
  {
    cand.mv_x = (short) (pic_pix_x + (p_UMHex->pred_MV_ref[0] / 4) * 4);
    cand.mv_y = (short) (pic_pix_y + (p_UMHex->pred_MV_ref[1] / 4) * 4);
    SEARCH_ONE_PIXEL
  }
  // Small local search
  iMinNow = best;
  for (m = 0; m < 4; m++)
  {
    cand.mv_x = iMinNow.mv_x + Diamond[m].mv_x;
    cand.mv_y = iMinNow.mv_y + Diamond[m].mv_y;
    SEARCH_ONE_PIXEL
  }

  //early termination algorithm, refer to JVT-G016
  EARLY_TERMINATION

    if(blocktype>6)
      goto fourth_1_step;
    else
      goto sec_step;

sec_step: //Unsymmetrical-cross search
  iMinNow = best;

  for(i = 4; i < search_range << 2; i+=8)
  {
    search_step = i;
    cand.mv_x = (short) (iMinNow.mv_x + search_step);
    cand.mv_y = iMinNow.mv_y ;
    SEARCH_ONE_PIXEL
      cand.mv_x = (short) (iMinNow.mv_x - search_step);
    cand.mv_y = iMinNow.mv_y ;
    SEARCH_ONE_PIXEL
  }
  for(i = 4; i < (search_range << 1);i+=8)
  {
    search_step = i;
    cand.mv_x = iMinNow.mv_x ;
    cand.mv_y = (short) (iMinNow.mv_y + search_step);
    SEARCH_ONE_PIXEL
      cand.mv_x = iMinNow.mv_x ;
    cand.mv_y = (short) (iMinNow.mv_y - search_step);
    SEARCH_ONE_PIXEL
  }


  //early termination alogrithm, refer to JVT-G016
  EARLY_TERMINATION

    iMinNow = best;

  //third_step:    // Uneven Multi-Hexagon-grid Search
  //sub step 1: 5x5 squre search
  for(pos=1;pos<25;pos++)
  {
    cand.mv_x = iMinNow.mv_x + p_Vid->spiral_qpel_search[pos].mv_x;
    cand.mv_y = iMinNow.mv_y + p_Vid->spiral_qpel_search[pos].mv_y;
    SEARCH_ONE_PIXEL
  }

  //early termination alogrithm, refer to JVT-G016
  EARLY_TERMINATION

    //sub step 2:  Multi-Hexagon-grid search
    memcpy(temp_Big_Hexagon_X,Big_Hexagon_X,64);
  memcpy(temp_Big_Hexagon_Y,Big_Hexagon_Y,64);
  for(i=1;i<=(search_range >> 2); i++)
  {

    for (m = 0; m < 16; m++)
    {
      cand.mv_x = (short) (iMinNow.mv_x + temp_Big_Hexagon_X[m]);
      cand.mv_y = (short) (iMinNow.mv_y + temp_Big_Hexagon_Y[m]);
      temp_Big_Hexagon_X[m] += Big_Hexagon_X[m];
      temp_Big_Hexagon_Y[m] += Big_Hexagon_Y[m];

      SEARCH_ONE_PIXEL
    }
    // ET_Thd2: early termination Threshold for strong motion
    if(min_mcost < ET_Thred)
    {
      goto terminate_step;
    }
  }


  //fourth_step:  //Extended Hexagon-based Search
  // the fourth step with a small search pattern
fourth_1_step:  //sub step 1: small Hexagon search
  for(i = 0; i < search_range; i++)
  {
    iMinNow = best;
    for (m = 0; m < 6; m++)
    {
      cand.mv_x = iMinNow.mv_x + Hexagon[m].mv_x;
      cand.mv_y = iMinNow.mv_y + Hexagon[m].mv_y;
      SEARCH_ONE_PIXEL
    }

    if (best.mv_x == iMinNow.mv_x && best.mv_y == iMinNow.mv_y)
    {
      break;
    }
  }
fourth_2_step: //sub step 2: small Diamond search

  for(i = 0; i < search_range; i++)
  {
    iMinNow = best;
    for (m = 0; m < 4; m++)
    {
      cand.mv_x = iMinNow.mv_x + Diamond[m].mv_x;
      cand.mv_y = iMinNow.mv_y + Diamond[m].mv_y;
      SEARCH_ONE_PIXEL
    }
    if(best.mv_x == iMinNow.mv_x && best.mv_y == iMinNow.mv_y)
      break;
  }

terminate_step:

  // store SAD infomation for prediction
  //FAST MOTION ESTIMATION. ZHIBO CHEN 2003.3
  for (i=0; i < (blocksize_x>>2); i++)
  {
    for (j=0; j < (blocksize_y>>2); j++)
    {
      if(mv_block->list == 0)
      {
        p_UMHex->fastme_ref_cost[ref][blocktype][block_y+j][block_x+i] = min_mcost;
        if (ref==0)
          p_UMHex->fastme_l0_cost[blocktype][(currMB->block_y)+block_y+j][(currMB->block_x)+block_x+i] = min_mcost;
      }
      else
      {
        p_UMHex->fastme_l1_cost[blocktype][(currMB->block_y)+block_y+j][(currMB->block_x)+block_x+i] = min_mcost;
      }
    }
  }
  //for multi ref SAD prediction
  if ((ref==0) || (SAD_prediction[pic_pix_x2] > min_mcost))
    SAD_prediction[pic_pix_x2] = min_mcost;

  mv->mv_x = (short) (best.mv_x - pic_pix_x);
  mv->mv_y = (short) (best.mv_y - pic_pix_y);
  return min_mcost;
}
