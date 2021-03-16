// offsets for defining location of RGB and YCC samples
#define RGB_RED		0
#define RGB_GREEN	1
#define RGB_BLUE	2
#define RGB_PIXELSIZE	3
#define R_Y_OFF		0		  
#define G_Y_OFF		(1*(MAXJSAMPLE+1))
#define B_Y_OFF		(2*(MAXJSAMPLE+1))
#define R_CB_OFF	(3*(MAXJSAMPLE+1))
#define G_CB_OFF	(4*(MAXJSAMPLE+1))
#define B_CB_OFF	(5*(MAXJSAMPLE+1))
#define R_CR_OFF	B_CB_OFF	
#define G_CR_OFF	(6*(MAXJSAMPLE+1))
#define B_CR_OFF	(7*(MAXJSAMPLE+1))
while (--num_rows >= 0) {
  // get input row of RGB
  inptr = *input_buf++;
  // get output rows for each of Y,Cb,Cr
  outptr0 = output_buf[0][output_row];
  outptr1 = output_buf[1][output_row];
  outptr2 = output_buf[2][output_row];
  output_row++;
  for (col = 0; col < num_cols; col++) {
    r = GETJSAMPLE(inptr[RGB_RED]);
    g = GETJSAMPLE(inptr[RGB_GREEN]);
    b = GETJSAMPLE(inptr[RGB_BLUE]);
    // move to the next grouping of samples
    inptr += RGB_PIXELSIZE;
    // do the color conversion
    outptr0[col] = (JSAMPLE)
	((ctab[r+R_Y_OFF] + ctab[g+G_Y_OFF] + ctab[b+B_Y_OFF])
	 >> SCALEBITS);
    outptr1[col] = (JSAMPLE)
	((ctab[r+R_CB_OFF] + ctab[g+G_CB_OFF] + ctab[b+B_CB_OFF])
	 >> SCALEBITS);
    outptr2[col] = (JSAMPLE)
	((ctab[r+R_CR_OFF] + ctab[g+G_CR_OFF] + ctab[b+B_CR_OFF])
	 >> SCALEBITS);
 }
}
