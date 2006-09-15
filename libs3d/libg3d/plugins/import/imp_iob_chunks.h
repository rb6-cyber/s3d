#ifndef _IMP_IOB_CHUNKS_H
#define _IMP_IOB_CHUNKS_H

#include <g3d/iff.h>

#include "imp_iob_callbacks.h"

static g3d_iff_chunk_info iob_chunks[] = {
	{ "AXIS", "coordinate system",                0, NULL },
	{ "BBOX", "bounding box data",                0, NULL },
	{ "BRS4", "brush (4)",                        0, NULL },
	{ "BRS5", "brush (5)",                        0, NULL },
	{ "COLR", "color",                            0, iob_cb_COLR },
	{ "CLS2", "color list (2)",                   0, iob_cb_xLSx },
	{ "CLST", "color list",                       0, iob_cb_xLSx },
	{ "DESC", "object description",               1, iob_cb_DESC },
	{ "EDG2", "edges",                            0, iob_cb_EDGx },
	{ "EDGE", "edges",                            0, iob_cb_EDGx },
	{ "EFLG", "edge flags",                       0, NULL },
	{ "FACE", "faces",                            0, iob_cb_FACx },
	{ "FAC2", "faces (2)",                        0, iob_cb_FACx },
	{ "FGR2", "face group (2)",                   0, NULL },
	{ "FGR3", "face group (3)",                   0, NULL },
	{ "FGR4", "face group (4)",                   0, NULL },
	{ "INT1", "light intensity (1)",              0, NULL },
	{ "NAME", "name",                             0, iob_cb_NAME },
	{ "OBJ ", "object",                           1, NULL },
	{ "PART", "particle parameters",              0, NULL },
	{ "PNT2", "points (2)",                       0, iob_cb_PNTx },
	{ "PNTS", "points",                           0, iob_cb_PNTx },
	{ "POSI", "position in world",                0, NULL },
	{ "PRP1", "properties (1)",                   0, NULL },
	{ "PRP2", "properties (2)",                   0, NULL },
	{ "REFL", "reflection",                       0, iob_cb_REFL },
	{ "RLS2", "reflection list (2)",              0, iob_cb_xLSx },
	{ "RLST", "reflection list",                  0, iob_cb_xLSx },
	{ "SPC1", "specularity (1)",                  0, NULL },
	{ "SPC2", "specularity (2)",                  0, NULL },
	{ "SHAP", "shape type",                       0, NULL },
	{ "SHP2", "shape type (2)",                   0, NULL },
	{ "SIZE", "size",                             0, NULL },
	{ "TLS2", "transparency list",                0, iob_cb_xLSx },
	{ "TLST", "transparency list (2)",            0, iob_cb_xLSx },
	{ "TOBJ", "end of object",                    0, NULL },
	{ "TRAN", "transparency",                     0, iob_cb_TRAN },
	{ "TXT3", "texture data (3)",                 0, NULL },
	{ "TXT4", "texture data (4)",                 0, NULL },

	{ NULL, NULL, 0, NULL }
};

#endif /* _IMP_IOB_CHUNKS_H */
