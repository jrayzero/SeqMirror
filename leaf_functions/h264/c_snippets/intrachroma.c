void intra_chroma_prediction (Macroblock *currMB, int *mb_up, int *mb_left, int*mb_up_left)
{
  int s, i, j;

  int      uv;
  int      b8, b4;
  imgpel   vline[16];

  int      mb_available_up;
  int      mb_available_left[2];
  int      mb_available_up_left;

  PixelPos pix_c;  //!< pixel position  p(0,-1)
  PixelPos pix_d;
  PixelPos pix_a;  //!< pixel positions p(-1, -1..15)

  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currSlice->p_Vid;
  InputParameters *p_Inp = currSlice->p_Inp;
  int      cr_MB_x = p_Vid->mb_cr_size_x;
  int      cr_MB_y = p_Vid->mb_cr_size_y;
  imgpel **cur_pred = NULL;

  imgpel *hline = NULL;

  int      yuv = p_Vid->yuv_format - 1;
  int      dc_pred_value_chroma = p_Vid->dc_pred_value_comp[1];
  int      max_imgpel_value_uv  = p_Vid->max_pel_value_comp[1];

  static const int block_pos[3][4][4]= //[yuv][b8][b4]
  {
    { {0, 1, 2, 3},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0}},
    { {0, 1, 2, 3},{2, 3, 2, 3},{0, 0, 0, 0},{0, 0, 0, 0}},
    { {0, 1, 2, 3},{1, 1, 3, 3},{2, 3, 2, 3},{3, 3, 3, 3}}
  };
  
  p_Vid->getNeighbour(currMB, -1, -1, p_Vid->mb_size[IS_CHROMA], &pix_d);
  p_Vid->getNeighbour(currMB, -1,  0, p_Vid->mb_size[IS_CHROMA], &pix_a);
  p_Vid->getNeighbour(currMB,  0, -1, p_Vid->mb_size[IS_CHROMA], &pix_c);

  mb_available_up      = pix_c.available;
  mb_available_up_left = pix_d.available;
  mb_available_left[0] = mb_available_left[1] = pix_a.available;

  if(p_Inp->UseConstrainedIntraPred)
  {
    mb_available_up      = pix_c.available ? p_Vid->intra_block[pix_c.mb_addr] : 0;
    mb_available_left[0] = mb_available_left[1] = pix_a.available ? p_Vid->intra_block[pix_a.mb_addr] : 0;
    mb_available_up_left = pix_d.available ? p_Vid->intra_block[pix_d.mb_addr] : 0;
  }

  if (mb_up)
    *mb_up = mb_available_up;
  if (mb_left)
    *mb_left = mb_available_left[0];
  if (mb_up_left)
    *mb_up_left = mb_available_up_left;

  // compute all chroma intra prediction modes for both U and V
  for (uv=0; uv<2; uv++)
  {
    imgpel **image = p_Vid->enc_picture->imgUV[uv];
    imgpel ***curr_mpr_16x16 = currSlice->mpr_16x16[uv + 1];

    // DC prediction
    for(b8=0; b8<p_Vid->num_blk8x8_uv >> 1;b8++)
    {
      for (b4 = 0; b4 < 4; b4++)
      {
        int block_y = subblk_offset_y[yuv][b8][b4];
        int block_x = subblk_offset_x[yuv][b8][b4];
        int blk_x = block_x;

        s = dc_pred_value_chroma;

        //===== get prediction value =====
        switch (block_pos[yuv][b8][b4])
        {
        case 0:  //===== TOP LEFT =====
          {
            int s0 = 0, s2 = 0;
            if (mb_available_up)       
            {
              int pos_x = pix_c.pos_x + blk_x;
              int pos_y = pix_c.pos_y;

              for (i = 0; i < BLOCK_SIZE; i++)  
                s0 += image[pos_y][pos_x++];
            }
            if (mb_available_left[0]) 
            {
              int pos_x = pix_a.pos_x;
              int pos_y = pix_a.pos_y + block_y; 

              for (i = 0; i < BLOCK_SIZE;i++)
                s2 += image[pos_y++][pos_x];
            }
            if (mb_available_up && mb_available_left[0])  
              s = (s0 + s2 + 4) >> 3;
            else if (mb_available_up)                          
              s = (s0 + 2) >> 2;
            else if (mb_available_left[0])                     
              s = (s2 + 2) >> 2;
          }
          break;
        case 1: //===== TOP RIGHT =====
          {
            int s1 = 0, s2 = 0;
            if (mb_available_up)
            {
              int pos_x = pix_c.pos_x + blk_x;
              int pos_y = pix_c.pos_y;
              for (i = 0; i < BLOCK_SIZE; i++)  
                s1 += image[pos_y][pos_x++];
            }
            else if (mb_available_left[0])
            {
              int pos_x = pix_a.pos_x;
              int pos_y = pix_a.pos_y + block_y; 

              for (i = 0; i < BLOCK_SIZE; i++)  
                s2 += image[pos_y++][pos_x];
            }
            if      (mb_available_up)       
              s  = (s1   +2) >> 2;
            else if (mb_available_left[0])                    
              s  = (s2   +2) >> 2;
          }
          break;
        case 2: //===== BOTTOM LEFT =====
          if      (mb_available_left[0])  
          {
            int pos_x = pix_a.pos_x;
            int pos_y = pix_a.pos_y + block_y; 
            int s3 = 0;

            for (i = 0; i < BLOCK_SIZE; i++)
              s3 += image[pos_y++][pos_x];
            s  = (s3 + 2) >> 2;
          }
          else if (mb_available_up)       
          {
            int pos_x = pix_c.pos_x + blk_x;
            int pos_y = pix_c.pos_y;

            int s0 = 0;
            for (i = 0; i < BLOCK_SIZE; i++)  
              s0 += image[pos_y][pos_x++];
            s  = (s0 + 2) >> 2;
          }
          break;
        case 3: //===== BOTTOM RIGHT =====
          {
            int s1 = 0, s3 = 0;
            if (mb_available_up)       
              for (i=blk_x;i<(blk_x+4);i++)  
                s1 += image[pix_c.pos_y][pix_c.pos_x + i];
            if (mb_available_left[0])  
            {
              int pos_x = pix_a.pos_x;
              int pos_y = pix_a.pos_y + block_y; 
              for (i = 0; i < BLOCK_SIZE;i++)  
                s3 += image[pos_y++][pos_x];
            }
            if      (mb_available_up && mb_available_left[0])  
              s  = (s1 + s3 + 4) >> 3;
            else if (mb_available_up)                          
              s  = (s1 + 2) >> 2;
            else if (mb_available_left[0])                     
              s  = (s3 + 2) >> 2;
          }
          break;
        }

        //===== prediction =====
        cur_pred = curr_mpr_16x16[DC_PRED_8];
        for (j = block_y; j < block_y+4; j++)
        {
          for (i = block_x; i < block_x+4; i++)
          {
            cur_pred[j][i] = (imgpel) s;
          }
        }
      }
    }

    // vertical prediction    
    if (mb_available_up)
    {
      cur_pred = curr_mpr_16x16[VERT_PRED_8];      
      hline = &image[pix_c.pos_y][pix_c.pos_x];
      for (j=0; j<cr_MB_y; j++)
        memcpy(cur_pred[j], hline, cr_MB_x * sizeof(imgpel));
    }

    // horizontal prediction
    if (mb_available_left[0])
    {
      int pos_x = pix_a.pos_x;
      int pos_y = pix_a.pos_y;
      cur_pred = curr_mpr_16x16[HOR_PRED_8];
      for (i=0; i<cr_MB_y; i++)
        vline[i] = image[pos_y++][pos_x];
      
      for (j=0; j<cr_MB_y; j++)
      {
        int predictor = vline[j];
        for (i = 0; i < cr_MB_x; i++)        
          cur_pred[j][i] = (imgpel) predictor;
      }
    }

    // plane prediction
    if (mb_available_left[0] && mb_available_up && mb_available_up_left)
    {
      int cr_x = (cr_MB_x >> 1);
      int cr_y = (cr_MB_y >> 1);

      int iaa, iv, ib, ic;
      int ih = cr_x * (hline[cr_MB_x-1] - image[pix_d.pos_y][pix_d.pos_x]);
      
      for (i = 0; i < cr_x - 1; i++)
        ih += (i + 1)*(hline[cr_x + i] - hline[cr_x - 2 - i]);

      iv = cr_y * (vline[cr_MB_y-1] - image[pix_d.pos_y][pix_d.pos_x]);
      for (i = 0; i < cr_y - 1; i++)
        iv += (i + 1) * (vline[cr_y + i] - vline[cr_y - 2 - i]);

      if (cr_MB_x == 8)
        ib = (17 * ih + 2 * cr_MB_x) >> 5;
      else
        ib = ( 5 * ih + 2 * cr_MB_x) >> 6;

      if (cr_MB_y == 8)
        ic = (17 * iv + 2 * cr_MB_y) >> 5;
      else
        ic = ( 5 * iv + 2 * cr_MB_y) >> 6;

      iaa = 16 * (hline[cr_MB_x - 1] + vline[cr_MB_y - 1]);
      cur_pred = curr_mpr_16x16[PLANE_8];
      iaa += (1 - cr_x) * ib + (1 - cr_y) * ic;
      for (j = 0; j < cr_MB_y; j++)
        for (i = 0; i < cr_MB_x; i++)
          cur_pred[j][i]= (imgpel) iClip1( max_imgpel_value_uv, (iaa + i * ib + j * ic + 16)>>5);
    }
  }

  if (!p_Inp->rdopt)      // the rd-opt part does not work correctly (see encode_one_macroblock)
  {                       // since ipredmodes could be overwritten => encoder-decoder-mismatches
    currSlice->rdo_low_intra_chroma_decision(currMB, mb_available_up, mb_available_left, mb_available_up_left);
  }
}
