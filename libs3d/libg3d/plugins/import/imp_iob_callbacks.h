#ifndef _IMP_IOB_CALLBACKS_H
#define _IMP_IOB_CALLBACKS_H

#include <g3d/iff.h>

gboolean iob_cb_xLSx(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean iob_cb_COLR(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean iob_cb_DESC(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean iob_cb_EDGx(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean iob_cb_FACx(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean iob_cb_NAME(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean iob_cb_PNTx(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean iob_cb_REFL(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean iob_cb_TRAN(g3d_iff_gdata *global, g3d_iff_ldata *local);

#endif /* _IMP_IOB_CALLBACKS_H */
