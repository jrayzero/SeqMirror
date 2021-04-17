distblk                                                  //  ==> minimum motion cost after search
EPZS_integer_motion_estimation (Macroblock * currMB,     // <--  current Macroblock
                                MotionVector * pred_mv,  // <--  motion vector predictor in sub-pel units
                                MEBlock * mv_block,      // <--  motion vector information
                                distblk min_mcost,       // <--  minimum motion cost (cost for center or huge value)
                                int lambda_factor        // <--  lagrangian parameter for determining motion cost
                                )
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  InputParameters *p_Inp = currMB->p_Inp;
  EPZSParameters *p_EPZS = currSlice->p_EPZS;
  PicMotionParams **motion = p_Vid->enc_picture->mv_info;

  int blocktype = mv_block->blocktype;

  int list = mv_block->list;
  int cur_list = list + currMB->list_offset;
  short ref = mv_block->ref_idx;
  MotionVector *mv = &mv_block->mv[list];
  SearchWindow *searchRange = &mv_block->searchRange;
  int mapCenter_x = searchRange->max_x - mv->mv_x;
  int mapCenter_y = searchRange->max_y - mv->mv_y;
  StorablePicture *ref_picture = currSlice->listX[cur_list][ref];

  distblk lambda_dist = weighted_cost(lambda_factor, 2);
  distblk stopCriterion = p_EPZS->medthres[blocktype] + lambda_dist;
  distblk *prevSad = &p_EPZS->distortion[cur_list][blocktype - 1][mv_block->pos_x2];

  MotionVector *p_motion = NULL;

  EPZSStructure *searchPatternF = p_EPZS->searchPattern;
  uint16 **EPZSMap = &p_EPZS->EPZSMap[mapCenter_y];
  uint16 *EPZSPoint = &p_EPZS->EPZSMap[searchRange->max_y][searchRange->max_x];

  MotionVector center = pad_MVs (*mv, mv_block);
  MotionVector pred = pad_MVs (*pred_mv, mv_block);
  MotionVector tmp = *mv, cand = center;

  ++p_EPZS->BlkCount;
  if (p_EPZS->BlkCount == 0)
    ++p_EPZS->BlkCount;

  if (p_Inp->EPZSSpatialMem)
  {
#if EPZSREF
    p_motion = &p_EPZS->p_motion[cur_list][ref][blocktype - 1][mv_block->block_y][mv_block->pos_x2];
#else
    p_motion = &p_EPZS->p_motion[cur_list][blocktype - 1][mv_block->block_y][mv_block->pos_x2];
#endif
  }

  // Clear EPZSMap
  // memset(EPZSMap[0],FALSE,searcharray*searcharray);
  // Check median candidate;
  //p_EPZS->EPZSMap[0][0] = p_EPZS->BlkCount;
  *EPZSPoint = p_EPZS->BlkCount;

  //--- initialize motion cost (cost for motion vector) and check ---
  min_mcost = mv_cost (p_Vid, lambda_factor, &cand, &pred);

  //--- add residual cost to motion cost ---
  min_mcost += mv_block->computePredFPel (ref_picture, mv_block, DISTBLK_MAX - min_mcost, &cand);


  // Additional threshold for ref>0
  //if ((ref > 0 && currSlice->structure == FRAME) && (*prevSad < imin (p_EPZS->medthres[blocktype] + lambda_dist, min_mcost))) 
  if ((ref > 0 && currSlice->structure == FRAME) && 
    ((*prevSad < distblkmin (p_EPZS->medthres[blocktype] + lambda_dist, min_mcost)) || (*prevSad * 8 < min_mcost)))
  {
#if EPZSREF
    if (p_Inp->EPZSSpatialMem)
#else 
    if (p_Inp->EPZSSpatialMem && ref == 0)
#endif 
    {
      *p_motion = tmp;
    }
    return min_mcost;
  }

  //! If p_EPZS->medthres satisfied, then terminate, otherwise generate Predictors
  //! Condition could be strengthened by consideration distortion of adjacent partitions.
  if (min_mcost > stopCriterion)
  {
    SPoint *p_EPZS_point = p_EPZS->predictor->point;
    Boolean checkMedian = FALSE;
    distblk second_mcost = DISTBLK_MAX;
    distblk mcost;
    int prednum = 5;
    int conditionEPZS;
    MotionVector tmp2 = {0, 0}, tmv;
    int pos;
    short invalid_refs = 0;

    stopCriterion = EPZSDetermineStopCriterion (p_EPZS, prevSad, mv_block, lambda_dist);

    if (min_mcost < (stopCriterion >> 1))
    {
#if EPZSREF
      if (p_Inp->EPZSSpatialMem)
#else 
      if (p_Inp->EPZSSpatialMem && ref == 0)
#endif 
      {
        *p_motion = tmp;
      }

      // This seems ok to add since this is considered a good enough position
      if ((ref == 0) || (*prevSad > min_mcost))
        *prevSad = min_mcost;

      return min_mcost;
    }

    //! Add Spatial Predictors in predictor list.
    //! Scheme adds zero, left, top-left, top, top-right. Note that top-left adds very little
    //! in terms of performance and could be removed with little penalty if any.
    invalid_refs = EPZS_spatial_predictors (p_EPZS, mv_block, 
      list, currMB->list_offset, ref, motion);

    if (p_Inp->EPZSSpatialMem)
      EPZS_spatial_memory_predictors (p_EPZS, mv_block, cur_list, &prednum, ref_picture->size_x >> 2);

    //if (p_Inp->HMEEnable == 1 && p_Inp->EPZSUseHMEPredictors == 1 && blocktype == 4)
    //if (p_Inp->HMEEnable == 1 && p_Inp->EPZSUseHMEPredictors == 1 && (currSlice->slice_type == P_SLICE || currSlice->slice_type == SP_SLICE || p_Inp->EnableReorderBslice) )
    if (p_Inp->HMEEnable == 1 && p_Inp->EPZSUseHMEPredictors == 1)
      EPZS_hierarchical_predictors (p_EPZS, mv_block, &prednum, ref_picture, currSlice);

    // Temporal predictors
#if (MVC_EXTENSION_ENABLE)
    if ( p_Inp->EPZSTemporal[currSlice->view_id] )
#else
    // Temporal predictors
    if (p_Inp->EPZSTemporal)
#endif
    {
      EPZS_temporal_predictors (currMB, ref_picture, p_EPZS, mv_block, &prednum, stopCriterion, min_mcost);
    }

    //! Window Size Based Predictors
    //! Basically replaces a Hierarchical ME concept and helps escaping local minima, or
    //! determining large motion variations.
    //! Following predictors can be adjusted further (i.e. removed, conditioned etc)
    //! based on distortion, correlation of adjacent MVs, complexity etc. These predictors
    //! and their conditioning could also be moved after all other predictors have been
    //! tested. Adaptation could also be based on type of material and coding mode (i.e.
    //! field/frame coding,MBAFF etc considering the higher dependency with opposite parity field
    //conditionEPZS = ((min_mcost > stopCriterion)
    // && (p_Inp->EPZSFixed > 1 || (p_Inp->EPZSFixed && currSlice->slice_type == P_SLICE)));
    //conditionEPZS = ((ref == 0) && (min_mcost > stopCriterion)
    //&& (p_Inp->EPZSFixed > 1 || (p_Inp->EPZSFixed && currSlice->slice_type == P_SLICE)));
    //conditionEPZS = ((min_mcost > stopCriterion) && ((ref < 2 && blocktype < 4)
    conditionEPZS = (p_Inp->EPZSFixed == 3 && (currMB->mb_x == 0 || currMB->mb_y == 0))
      || ((min_mcost > 3 * stopCriterion) && ((ref < 2 && blocktype < 4) || (ref < 1 && blocktype == 4)      
      || ((currSlice->structure != FRAME || currMB->list_offset)
      && ref < 3))
      && (p_Inp->EPZSFixed > 1 || (p_Inp->EPZSFixed && currSlice->slice_type == P_SLICE)));

    if (conditionEPZS)
      EPZSWindowPredictors (mv, p_EPZS->predictor, &prednum, 
      (p_Inp->EPZSAggressiveWindow != 0) || ((invalid_refs > 2) && (ref < 1 + (currSlice->structure != FRAME || currMB->list_offset)))
      ? p_EPZS->window_predictor_ext : p_EPZS->window_predictor);

    //! Blocktype/Reference dependent predictors.
    //! Since already mvs for other blocktypes/references have been computed, we can reuse
    //! them in order to easier determine the optimal point. Use of predictors could depend
    //! on cost,
    //conditionEPZS = (ref == 0 || (ref > 0 && min_mcost > stopCriterion) || currSlice->structure != FRAME || currMB->list_offset);
    conditionEPZS = (ref == 0 || (ref > 0 && min_mcost > 2 * stopCriterion));

    if (conditionEPZS && currMB->mbAddrX != 0 && p_Inp->EPZSBlockType)
      EPZSBlockTypePredictorsMB (currSlice, mv_block, p_EPZS_point, &prednum);

    //! Check all predictors
    for (pos = 0; pos < prednum; ++pos)
    {
      tmv = p_EPZS_point[pos].motion;
      //if (((iabs (tmv.mv_x - mv->mv_x) > searchRange->max_x || iabs (tmv.mv_y - mv->mv_y) > searchRange->max_y)) && (tmv.mv_x || tmv.mv_y))
      if ((iabs (tmv.mv_x - mv->mv_x) - searchRange->max_x <= 0) && (iabs (tmv.mv_y - mv->mv_y) - searchRange->max_y <= 0))
      {
        EPZSPoint = &EPZSMap[tmv.mv_y][mapCenter_x + tmv.mv_x];
        if (*EPZSPoint != p_EPZS->BlkCount)
        {
          *EPZSPoint = p_EPZS->BlkCount;
          cand = pad_MVs (tmv, mv_block);

          //--- set motion cost (cost for motion vector) and check ---
          mcost = mv_cost (p_Vid, lambda_factor, &cand, &pred);

          if (mcost < second_mcost)
          {
            mcost += mv_block->computePredFPel (ref_picture, mv_block, second_mcost - mcost, &cand);

            //--- check if motion cost is less than minimum cost ---
            if (mcost < min_mcost)
            {
              tmp2 = tmp;
              tmp = tmv;
              second_mcost = min_mcost;
              min_mcost = mcost;
              checkMedian = TRUE;
            }
            //else if (mcost < second_mcost && (tmp.mv_x != tmv.mv_x || tmp.mv_y != tmv.mv_y))
            else if (mcost < second_mcost)
            {
              tmp2 = tmv;
              second_mcost = mcost;
              checkMedian = TRUE;
            }
          }
        }
      }
    }

    if ((ref > 0 && currSlice->structure == FRAME) && (*prevSad * 3 < min_mcost))
    {  
#if EPZSREF
      if (p_Inp->EPZSSpatialMem)
#else 
      if (p_Inp->EPZSSpatialMem && ref == 0)
#endif 
      {
        *p_motion = tmp;
      }

      *mv = tmp;

     // this condition seems redundant since (*prevSad * 3 < min_mcost)
      //if ((*prevSad > min_mcost))
        // *prevSad = min_mcost;

      return min_mcost;
    }

    //! Refine using EPZS pattern if needed
    //! Note that we are using a conservative threshold method. Threshold
    //! could be tested after checking only a certain number of predictors
    //! instead of the full set. Code could be easily modified for this task.
    if (min_mcost > stopCriterion)
    {
      const int mv_range = 10;
      int patternStop = 0, pointNumber = 0, checkPts, nextLast = 0;
      int totalCheckPts = 0, motionDirection = 0;

      //! Adapt pattern based on different conditions.
      if (p_Inp->EPZSPattern != 0)
      {
        if ((min_mcost < stopCriterion + ((3 * p_EPZS->medthres[blocktype]) >> 1)))
        {
          if ((tmp.mv_x == 0 && tmp.mv_y == 0) 
            || (iabs (tmp.mv_x - mv->mv_x) < (mv_range) && iabs (tmp.mv_y - mv->mv_y) < (mv_range)))
            searchPatternF = p_Vid->sdiamond;
          else
            searchPatternF = p_Vid->square;
        }
        else if (ref > 0 && blocktype != 1)
          searchPatternF = p_Vid->square;
        else
          searchPatternF = p_EPZS->searchPattern;
      }

      //! center on best predictor
      center = tmp;

      for (;;)
      {
        totalCheckPts = searchPatternF->searchPoints;
        do
        {
          checkPts = totalCheckPts;
          do
          {
            tmv = add_MVs (center, &(searchPatternF->point[pointNumber].motion));

            if (((iabs (tmv.mv_x - mv->mv_x) - searchRange->max_x) <= 0) && ((iabs (tmv.mv_y - mv->mv_y) - searchRange->max_y) <= 0))
            {
              EPZSPoint = &EPZSMap[tmv.mv_y][mapCenter_x + tmv.mv_x];
              if (*EPZSPoint != p_EPZS->BlkCount)
              {
                *EPZSPoint = p_EPZS->BlkCount;
                cand = pad_MVs (tmv, mv_block);

                mcost = mv_cost (p_Vid, lambda_factor, &cand, &pred);

                if (mcost < min_mcost)
                {
                  mcost += mv_block->computePredFPel (ref_picture, mv_block, min_mcost - mcost, &cand);

                  if (mcost < min_mcost)
                  {
                    tmp = tmv;
                    min_mcost = mcost;
                    motionDirection = pointNumber;
                  }
                }
              }
            }
            ++pointNumber;
            if (pointNumber >= searchPatternF->searchPoints)
              pointNumber -= searchPatternF->searchPoints;
            checkPts--;
          }
          while (checkPts > 0);

          if (nextLast || ((tmp.mv_x == center.mv_x) && (tmp.mv_y == center.mv_y)))
          {
            patternStop = searchPatternF->stopSearch;
            searchPatternF = searchPatternF->nextpattern;
            totalCheckPts = searchPatternF->searchPoints;
            nextLast = searchPatternF->nextLast;
            motionDirection = 0;
            pointNumber = 0;
          }
          else
          {
            totalCheckPts = searchPatternF->point[motionDirection].next_points;
            pointNumber = searchPatternF->point[motionDirection].start_nmbr;
            center = tmp;
          }
        }
        while (patternStop != 1);

        if ((ref > 0) && (currSlice->structure == FRAME) 
          && ((4 * *prevSad < min_mcost) || ((3 * *prevSad < min_mcost) && (*prevSad <= stopCriterion))))
        {
          *mv = tmp;
#if EPZSREF
          if (p_Inp->EPZSSpatialMem)
#else  
          if (p_Inp->EPZSSpatialMem && ref == 0)
#endif  
          {
            *p_motion = tmp;
          }

          return min_mcost;
        }

        //! Check Second best predictor with EPZS pattern
        conditionEPZS = (checkMedian == TRUE)
          && (ref == 0 || (ref > 0 && min_mcost < 2 * *prevSad))
          && (min_mcost > (( 3 * stopCriterion) >> 1)) && (p_Inp->EPZSDual > 0);

        if (!conditionEPZS)
          break;

        pointNumber = 0;
        patternStop = 0;
        motionDirection = 0;
        nextLast = 0;

        if ((tmp.mv_x == 0 && tmp.mv_y == 0) || (tmp.mv_x == mv->mv_x && tmp.mv_y == mv->mv_y))
        {
          if (iabs (tmp.mv_x - mv->mv_x) < (mv_range) && iabs (tmp.mv_y - mv->mv_y) < (mv_range))
            searchPatternF = p_Vid->sdiamond;
          else
            searchPatternF = p_Vid->square;
        }
        else
          searchPatternF = p_EPZS->searchPatternD;

        //! Second best. Note that following code is identical as for best predictor.
        center = tmp2;
        checkMedian = FALSE;
      }
    }
  }

  if ((ref == 0) || (*prevSad > min_mcost))
    *prevSad = min_mcost;
#if EPZSREF
  if (p_Inp->EPZSSpatialMem)
#else  
  if (p_Inp->EPZSSpatialMem && ref == 0)
#endif  
  {
    *p_motion = tmp;
    //printf("value %d %d %d %d\n", p_motion->mv_x, p_motion->mv_y, p_motion[cur_list][ref][0][0][0].mv_x, p_motion[list + list_offset][ref][0][0][0].mv_y);
    //printf("xxxxx %d %d %d %d\n", p_motion->mv_x, p_motion->mv_y, p_motion[cur_list][ref][blocktype - 1][mv_block->block_y][pic_pix_x2].mv_x, p_motion[cur_list][ref][blocktype - 1][mv_block->block_y][pic_pix_x2].mv_y);
  }

  *mv = tmp;

  return min_mcost;
}


EPZS_spatial_predictors (EPZSParameters * p_EPZS, MEBlock *mv_block, int list, int list_offset, short ref, struct pic_motion_params **mv_info)
{
  PixelPos * block = mv_block->block;
  short refA = 0, refB = 0, refC = 0, refD = 0;
  VideoParameters *p_Vid = p_EPZS->p_Vid;
  int *mot_scale = p_EPZS->mv_scale[list + list_offset][ref];
  SPoint *point = p_EPZS->predictor->point;

  // zero predictor
  (point++)->motion = zero_mv;

  // Non MB-AFF mode
  if (!p_Vid->mb_aff_frame_flag)
  {
    refA = block[0].available ? (short) mv_info[block[0].pos_y][block[0].pos_x].ref_idx[list] : -1;
    refB = block[1].available ? (short) mv_info[block[1].pos_y][block[1].pos_x].ref_idx[list] : -1;
    refC = block[2].available ? (short) mv_info[block[2].pos_y][block[2].pos_x].ref_idx[list] : -1;
    refD = block[3].available ? (short) mv_info[block[3].pos_y][block[3].pos_x].ref_idx[list] : -1;

    // Left Predictor
    if (block[0].available)
    {
      scale_mv (&point->motion, mot_scale[refA], &mv_info[block[0].pos_y][block[0].pos_x].mv[list], 8);
      /*
      if (*((int*) &point->motion) == 0)
      {
      point->motion.mv_x = 12;
      point->motion.mv_y = 0;
      }
      */
      ++point;
    }
    else
    {
      (point)->motion.mv_x = 12;
      (point++)->motion.mv_y = 0;
    }
    // Up predictor
    if (block[1].available)
    {
      scale_mv (&point->motion, mot_scale[refB], &mv_info[block[1].pos_y][block[1].pos_x].mv[list], 8);
      /*
      if (*((int*) &point->motion) == 0)
      {
      point->motion.mv_x = 0;
      point->motion.mv_y = 12;
      }
      */
      ++point;
    }
    else
    {
      (point)->motion.mv_x = 0;
      (point++)->motion.mv_y = 12;
    }

    // Up-Right predictor
    if (block[2].available)
    {
      scale_mv (&point->motion, mot_scale[refC], &mv_info[block[2].pos_y][block[2].pos_x].mv[list], 8);
      /*
      if (*((int*) &point->motion) == 0)
      {
      point->motion.mv_x = -12;
      point->motion.mv_y = 0;
      }
      */
      ++point;
    }
    else
    {
      (point)->motion.mv_x = -12;
      (point++)->motion.mv_y = 0;
    }

    //Up-Left predictor
    if (block[3].available)
    {
      scale_mv (&point->motion, mot_scale[refD], &mv_info[block[3].pos_y][block[3].pos_x].mv[list], 8);
      /*
      if (*((int*) &point->motion) == 0)
      {
      point->motion.mv_x = 0;
      point->motion.mv_y = -12;
      }
      */
      ++point;
    }
    else
    {
      (point)->motion.mv_x = 0;
      (point++)->motion.mv_y = -12;
    }
  }  return ((refA == -1) + (refB == -1) + ((refC == -1) && (refD == -1)));
}