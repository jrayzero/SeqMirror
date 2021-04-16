            bcost <<= 4;
            int i = i_me_range;
            do
            {
                COST_MV_X4_DIR( 0,-1, 0,1, -1,0, 1,0, costs );
                COPY1_IF_LT( bcost, (costs[0]<<4)+1 );
                COPY1_IF_LT( bcost, (costs[1]<<4)+3 );
                COPY1_IF_LT( bcost, (costs[2]<<4)+4 );
                COPY1_IF_LT( bcost, (costs[3]<<4)+12 );
                if( !(bcost&15) )
                    break;
                bmx -= (bcost<<28)>>30;
                bmy -= (bcost<<30)>>30;
                bcost &= ~15;
            } while( --i && CHECK_MVRANGE(bmx, bmy) );
            bcost >>= 4;
            break;

            /* hexagon */
            COST_MV_X3_DIR( -2,0, -1, 2,  1, 2, costs   );
            COST_MV_X3_DIR(  2,0,  1,-2, -1,-2, costs+4 ); /* +4 for 16-byte alignment */
            bcost <<= 3;
            COPY1_IF_LT( bcost, (costs[0]<<3)+2 );
            COPY1_IF_LT( bcost, (costs[1]<<3)+3 );
            COPY1_IF_LT( bcost, (costs[2]<<3)+4 );
            COPY1_IF_LT( bcost, (costs[4]<<3)+5 );
            COPY1_IF_LT( bcost, (costs[5]<<3)+6 );
            COPY1_IF_LT( bcost, (costs[6]<<3)+7 );
            /* square refine */
            bcost <<= 4;
            COST_MV_X4_DIR(  0,-1,  0,1, -1,0, 1,0, costs );
            COPY1_IF_LT( bcost, (costs[0]<<4)+1 );
            COPY1_IF_LT( bcost, (costs[1]<<4)+2 );
            COPY1_IF_LT( bcost, (costs[2]<<4)+3 );
            COPY1_IF_LT( bcost, (costs[3]<<4)+4 );
            COST_MV_X4_DIR( -1,-1, -1,1, 1,-1, 1,1, costs );
            COPY1_IF_LT( bcost, (costs[0]<<4)+5 );
            COPY1_IF_LT( bcost, (costs[1]<<4)+6 );
            COPY1_IF_LT( bcost, (costs[2]<<4)+7 );
            COPY1_IF_LT( bcost, (costs[3]<<4)+8 );
            bmx += square1[bcost&15][0];
            bmy += square1[bcost&15][1];
            bcost >>= 4;
            break;
