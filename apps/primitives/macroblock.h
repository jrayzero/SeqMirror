#ifndef MACROBLOCK
#define MACROBLOCK
#include "common.h"

struct video {
    int height;
    int width;
    int planes;
    int nframes;
    int chroma_height;
    int chroma_width;;
    int *y_data;
    int *u_data;
    int *v_data;
};

struct macroblock {
    int mbAddrX;                    //!< current MB address
    int mb_x;                       //!< current MB horizontal
    int mb_y;                       //!< current MB vertical
    int block_x;                    //!< current block horizontal
    int block_y;                    //!< current block vertical
    int pix_x;                      //!< current pixel horizontal
    int pix_y;                      //!< current pixel vertical
    int pix_c_x;                    //!< current pixel chroma horizontal
    int pix_c_y;                    //!< current pixel chroma vertical
    int subblock_x;                 //!< current subblock horizontal
    int subblock_y;                 //!< current subblock vertical
    int mbAddrA, mbAddrB, mbAddrC, mbAddrD;
    unsigned char mbAvailA, mbAvailB, mbAvailC, mbAvailD;

    int **intra16x16_pred;
    int **intra16x16_pred_buf[2];    
    // ptr to the video data
    struct video *vid;
};

#endif
