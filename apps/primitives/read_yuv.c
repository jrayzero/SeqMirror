#include "macroblock.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct video *ingest(const char *fn, int height, int width, int nframes, struct video *vid) {
    FILE *fd = fopen(fn, "r");
    assert(fd);
    int padded_height = ((height & 15) == 0) ? height : height + 16 - (height & 15);
    int padded_width = ((width & 15) == 0) ? width : width + 16 - (width & 15);
    DO(fprintf(stderr, "padded_height %d, padded_width %d\n", padded_height, padded_width);)	
    vid->height = padded_height;
    vid->width = padded_width;
    vid->nframes = nframes;
    vid->chroma_height = height/2;
    vid->chroma_width = width/2;
    vid->y_data = (int*)malloc(sizeof(int) * padded_height * padded_width * nframes);
    // for 420, have half the width and height for chroma
    vid->u_data = (int*)malloc(sizeof(int) * height/2 * width/2 * nframes);
    vid->v_data = (int*)malloc(sizeof(int) * height/2 * width/2 * nframes);

    // can only read one value at a time here because I'm upcasting to int
    // DO NOT USE MULTIPLE BYTES. YOU WILL READ THE WRONG VALUES
    for (int i = 0; i < nframes; i++) {
	// Y
	// fill each row
	for (int r = 0; r < height; r++) {
	    int start = i * padded_height * padded_width + r * padded_width;
	    for (int w = 0; w < width; w++) {
		assert(fread(&vid->y_data[start+w], 1, 1, fd) == 1);
	    }
	    // pad to the right
	    for (int c = width; c < padded_width; c++) {
		vid->y_data[start + c] = vid->y_data[start + width - 1];
	    }
	}
	// pad below, whole row at a time
	for (int r = height; r < padded_height; r++) {
	    int start = i * padded_height * padded_width + r * padded_width;
	    memcpy(&vid->y_data[start], 
		   &vid->y_data[i * padded_height * padded_width + (height-1) * padded_width], padded_width*sizeof(int));
	}
	// U
	int r = 0;
	for (r = 0; r < height/2*width/2; r++) {
	    assert(fread(&vid->u_data[i * height/2 * width/2 + r], 1, 1, fd) == 1);
	}
	// V
	for (r = 0; r < height/2*width/2; r++) {
	    assert(fread(&vid->v_data[i * height/2 * width/2 + r], 1, 1, fd) == 1);
	}
    }    
    fclose(fd);
    return vid;
}
