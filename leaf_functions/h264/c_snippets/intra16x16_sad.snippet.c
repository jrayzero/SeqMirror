distblk distI16x16_satd(Macroblock *currMB, imgpel **img_org, imgpel **pred_img, distblk min_cost)
{
  Slice *currSlice = currMB->p_Slice;
  int   **M7 = NULL;
  int   **tblk4x4 = currSlice->tblk4x4;
  int   ****i16blk4x4 = currSlice->i16blk4x4;
  imgpel *cur_img, *prd_img;
  distblk current_intra_sad_2 = 0;
  int ii, jj, i, j, i32Cost = 0;
  int imin_cost = dist_down(min_cost);

  for (j = 0; j < MB_BLOCK_SIZE; j++)
  {
    cur_img = &img_org[currMB->opix_y + j][currMB->pix_x];
    prd_img = pred_img[j];
    for (i = 0; i < MB_BLOCK_SIZE; i++)
    {
      i16blk4x4[j >> 2][i >> 2][j & 0x03][i & 0x03] = cur_img[i] - prd_img[i];
    }
  }

  for (jj = 0; jj < 4; jj++)
  {
    for (ii = 0; ii < 4;ii++)
    {
      M7 = i16blk4x4[jj][ii];
      hadamard4x4(M7, M7);
      i32Cost += iabs(M7[0][1]);
      i32Cost += iabs(M7[0][2]);
      i32Cost += iabs(M7[0][3]);

      if (i32Cost > imin_cost)
        return (min_cost);

      for (j = 1; j < 4; j++)
      {
        //i32Cost =0;
        i32Cost += iabs(M7[j][0]);
        i32Cost += iabs(M7[j][1]);
        i32Cost += iabs(M7[j][2]);
        i32Cost += iabs(M7[j][3]);

        if (i32Cost > imin_cost)
          return (min_cost);
      }
    }
  }

  for (j = 0; j < 4;j++)
  {
    tblk4x4[j][0] = (i16blk4x4[j][0][0][0] >> 1);
    tblk4x4[j][1] = (i16blk4x4[j][1][0][0] >> 1);
    tblk4x4[j][2] = (i16blk4x4[j][2][0][0] >> 1);
    tblk4x4[j][3] = (i16blk4x4[j][3][0][0] >> 1);     
  }

  // Hadamard of DC coeff
  hadamard4x4(tblk4x4, tblk4x4);

  for (j = 0; j < 4; j++)
  {
    i32Cost += iabs(tblk4x4[j][0]);
    i32Cost += iabs(tblk4x4[j][1]);
    i32Cost += iabs(tblk4x4[j][2]);
    i32Cost += iabs(tblk4x4[j][3]);

    if (i32Cost > imin_cost)
      return (min_cost);
  }

  current_intra_sad_2 += (dist_scale((distblk)i32Cost));
  return current_intra_sad_2;
}

distblk distI16x16_sad(Macroblock *currMB, imgpel **img_org, imgpel **pred_img, distblk min_cost)
{
  imgpel *cur_img, *prd_img;
  int i32Cost = 0;
  int i, j; 
  int imin_cost = dist_down(min_cost);

  for (j = 0; j < MB_BLOCK_SIZE; j++)
  {
    cur_img = &img_org[currMB->opix_y + j][currMB->pix_x];
    prd_img = pred_img[j];
    for (i = 0; i < MB_BLOCK_SIZE; i++)
    {
      i32Cost += iabs( *cur_img++ - *prd_img++ );
    }

    if (i32Cost > imin_cost)
      return (min_cost);
  }

  return (dist_scale((distblk) i32Cost));
}

distblk distI16x16_sse(Macroblock *currMB, imgpel **img_org, imgpel **pred_img, distblk min_cost)
{
  imgpel *cur_img, *prd_img;  
  int i, j, i32Cost = 0; 
  int imin_cost = dist_down(min_cost);

  for (j = 0; j < MB_BLOCK_SIZE;j++)
  {
    cur_img = &img_org[currMB->opix_y + j][currMB->pix_x];
    prd_img = pred_img[j];
    for (i = 0; i < MB_BLOCK_SIZE; i++)
    {
      i32Cost += iabs2( *cur_img++ - *prd_img++ );
    }

    if (i32Cost > imin_cost)
      return (min_cost);
  }

  return (dist_scale((distblk) i32Cost));
}


/*!
 ************************************************************************
 * \brief
 *    Find best 16x16 based intra mode
 *
 * \par Input:
 *    Image parameters, pointer to best 16x16 intra mode
 *
 * \par Output:
 *    best 16x16 based SAD
 ************************************************************************/
distblk find_sad_16x16_JM(Macroblock *currMB)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  InputParameters *p_Inp = currMB->p_Inp;
  distblk current_intra_sad_2, best_intra_sad2 = DISTBLK_MAX;
  int k;
  imgpel  ***curr_mpr_16x16 = currSlice->mpr_16x16[0];

  int up_avail, left_avail, left_up_avail;

  currMB->i16mode = DC_PRED_16;
  
  currSlice->set_intrapred_16x16(currMB, PLANE_Y, &left_avail, &up_avail, &left_up_avail);
  // For speed purposes, we should just unify all planes
  if (currSlice->P444_joined)
  {
    currSlice->set_intrapred_16x16(currMB, PLANE_U, &left_avail, &up_avail, &left_up_avail);
    currSlice->set_intrapred_16x16(currMB, PLANE_V, &left_avail, &up_avail, &left_up_avail);
  }

  for (k = VERT_PRED_16; k <= PLANE_16; k++)
  {
    if (p_Inp->IntraDisableInterOnly == 0 || (currSlice->slice_type != I_SLICE && currSlice->slice_type != SI_SLICE) )
    {
      if (p_Inp->Intra16x16ParDisable && (k == VERT_PRED_16||k == HOR_PRED_16))
        continue;

      if (p_Inp->Intra16x16PlaneDisable && k == PLANE_16)
        continue;
    }
    //check if there are neighbours to predict from
    if (!((k == VERT_PRED_16 && !up_avail) || (k == HOR_PRED_16 && !left_avail) || (k == PLANE_16 && (!left_avail || !up_avail || !left_up_avail))))
    {
      get_intrapred_16x16(currMB, PLANE_Y, k, left_avail, up_avail);
      current_intra_sad_2 = currSlice->distI16x16(currMB, p_Vid->pCurImg, curr_mpr_16x16[k], best_intra_sad2);
      if (currSlice->P444_joined)
      {
        get_intrapred_16x16(currMB, PLANE_U, k, left_avail, up_avail);
        current_intra_sad_2 += currSlice->distI16x16(currMB, p_Vid->pImgOrg[1], currSlice->mpr_16x16[1][k], best_intra_sad2);
        get_intrapred_16x16(currMB, PLANE_V, k, left_avail, up_avail);
        current_intra_sad_2 += currSlice->distI16x16(currMB, p_Vid->pImgOrg[2], currSlice->mpr_16x16[2][k], best_intra_sad2);
      }

      if (current_intra_sad_2 < best_intra_sad2)
      {
        best_intra_sad2 = current_intra_sad_2;
        currMB->i16mode = (char) k; // update best intra mode
      }
    }    
  }

  return best_intra_sad2;
}

