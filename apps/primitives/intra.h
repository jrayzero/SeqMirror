#ifndef INTRA
#define INTRA

void JM_intra16x16_luma_vertical(imgpel **cur_pred, imgpel *PredPel);
void JM_intra16x16_luma_horizontal(imgpel **cur_pred, imgpel *PredPel);
void JM_intra16x16_luma_dc(imgpel **cur_pred, imgpel *PredPel, int left_available, int up_available);
void JM_intra16x16_luma_plane(imgpel **cur_pred, imgpel *PredPel, int max_imgpel_value);
void JM_chroma_vertical(imgpel **cur_pred, imgpel *PredPel, int cr_MB_x, int cr_MB_y);
void JM_chroma_horizontal(imgpel **cur_pred, imgpel *PredPel, int cr_MB_x, int cr_MB_y);
void JM_chroma_dc(imgpel **cur_pred, imgpel *PredPel, int left_available, int up_available);
void JM_chroma_plane(imgpel **cur_pred, imgpel *PredPel, int max_imgpel_value);

#endif
