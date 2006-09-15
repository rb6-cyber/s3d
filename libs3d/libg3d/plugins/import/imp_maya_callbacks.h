#ifndef _IMP_MAYA_CALLBACKS_H
#define _IMP_MAYA_CALLBACKS_H

#include <g3d/iff.h>

gboolean maya_cb_CMPD(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean maya_cb_CREA(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean maya_cb_DBLn(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean maya_cb_DBL2(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean maya_cb_DBL3(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean maya_cb_DBLE(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean maya_cb_DMSH(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean maya_cb_FLT3(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean maya_cb_MESH(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean maya_cb_PCUB(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean maya_cb_STR_(g3d_iff_gdata *global, g3d_iff_ldata *local);
gboolean maya_cb_XFRM(g3d_iff_gdata *global, g3d_iff_ldata *local);

#endif /* _IMP_MAYA_CALLBACKS_H */
