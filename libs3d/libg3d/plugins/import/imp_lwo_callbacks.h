#ifndef _IMP_LWO_CALLBACKS_H
#define _IMP_LWO_CALLBACKS_H

gboolean lwo_cb_CLIP(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean lwo_cb_COLR(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean lwo_cb_IMAG(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean lwo_cb_PNTS(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean lwo_cb_POLS(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean lwo_cb_PTAG(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean lwo_cb_SPEC(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean lwo_cb_SRFS(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean lwo_cb_STIL(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean lwo_cb_SURF(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean lwo_cb_TAGS(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean lwo_cb_TRAN(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean lwo_cb_VMAP(g3d_iff_gdata *global, g3d_iff_ldata *local);

#endif
