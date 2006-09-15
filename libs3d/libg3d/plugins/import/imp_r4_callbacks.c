#include <g3d/iff.h>
#include <g3d/read.h>

#include "imp_r4_chunks.h"

/* camera related */
gboolean r4_cb_RKA2(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	guint32 chunk_id, chunk_len;
	g3d_iff_ldata *sublocal;

	/* RGE1 chunk */
	g3d_iff_readchunk(global->f, &chunk_id, &chunk_len, 0);
	local->nb -= 8;

	sublocal = g_new0(g3d_iff_ldata, 1);
	sublocal->parent_id = local->id;
	sublocal->id = chunk_id;
	sublocal->object = local->object;
	sublocal->level = local->level + 1;
	sublocal->nb = chunk_len;
	g3d_iff_read_ctnr(global, sublocal, r4_chunks, G3D_IFF_PAD1);
	g_free(sublocal);

	/* more stuff... */
	/* TODO: */

	return TRUE;
}

/* object name */
gboolean r4_cb_ROBJ(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	gchar buffer[512];
	gint32 len;

	len = g3d_read_int16_be(global->f);
	local->nb -= 2;

	fread(buffer, 1, len, global->f);
	local->nb -= len;
	buffer[len] = '\0';

#if DEBUG > 0
	g_printerr("[R4] ROBJ: %s\n", buffer);
#endif

	return TRUE;
}
