#include "macroblock.h"
#include "ticky.h"
#include "read_yuv.h"
#include <stdlib.h>
#include <stdio.h>

int main() {
    const char *input_file = "foreman_part_qcif_x10.yuv";
    int nframes = 3;
    int height = 144;
    int width = 176;
    struct video *vid = (struct video*)malloc(sizeof(struct video));
#ifdef TIME
    int niters = 10;
    struct timespec tick = ctick();
    for (int n = 0; n < niters; n++) {
#endif
    ingest(input_file, height, width, nframes, vid);
#ifdef TIME
    }
    ctock(tick, niters);
#endif
#ifdef DEBUG
    FILE *fd = fopen("yuv.c.out", "w");
    fprintf(fd, "Y %d %d\n", vid->height, vid->width);
    int z = 0;
    for (int i = 0; i < vid->nframes; i++) {
	for (int j = 0; j < vid->height; j++) {
	    for (int k = 0; k < vid->width; k++) {
		fprintf(fd, "%d ", vid->y_data[z]);
		z++;
	    }
	    fprintf(fd, "\n");
	}
	fprintf(fd, "\n");
    }
    fprintf(fd, "U %d %d\n", vid->height/2, vid->width/2);
    z = 0;
    for (int i = 0; i < vid->nframes; i++) {
	for (int j = 0; j < vid->chroma_height; j++) {
	    for (int k = 0; k < vid->chroma_width; k++) {
		fprintf(fd, "%d ", vid->u_data[z]);
		z++;
	    }
	    fprintf(fd, "\n");
	}
	fprintf(fd, "\n");
    }
    fprintf(fd, "V %d %d\n", vid->height/2, vid->width/2);
    z = 0;
    for (int i = 0; i < vid->nframes; i++) {
	for (int j = 0; j < vid->chroma_height; j++) {
	    for (int k = 0; k < vid->chroma_width; k++) {
		fprintf(fd, "%d ", vid->v_data[z]);
		z++;
	    }
	    fprintf(fd, "\n");
	}
	fprintf(fd, "\n");
    }
    fflush(fd);
    fclose(fd);
#endif
    free(vid->y_data);
    free(vid->u_data);
    free(vid->v_data);
}
