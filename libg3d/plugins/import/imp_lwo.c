/* $Id: imp_lwo.c,v 1.1.2.3 2006/01/23 17:03:06 dahms Exp $ */

/*
    libg3d - 3D object loading library

    Copyright (C) 2005, 2006  Markus Dahms <mad@automagically.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*
 *  imp_lwo.c - LightWave import plugin
 *
 *  (C) 2005,2006 Markus Dahms
 *
 *  based on gtkglarea example viewlw:
 *  Copyright (C) 1998 Janne Löf <jlof@mail.student.oulu.fi>
 */

#include <string.h>

#include <g3d/types.h>
#include <g3d/context.h>
#include <g3d/material.h>
#include <g3d/read.h>
#include <g3d/iff.h>

#define LW_MAX_POINTS   200
#define LW_MAX_NAME_LEN 500

#define LW_F_LWO2		(1 << 0)

static gboolean lwo_read_directory(FILE *f, guint32 nbytes, G3DModel *model,
	gpointer pobject, guint32 parentid, guint32 level, guint32 flags,
	G3DContext *context);

/*****************************************************************************/
/* plugin interface                                                          */
/*****************************************************************************/

gboolean plugin_load_model(G3DContext *context, const gchar *filename,
	G3DModel *model, gpointer user_data)
{
	G3DObject *lwo_object = NULL;
	G3DMaterial *material;
	FILE *f;
	guint32 id, len, flags = 0;

	f = g3d_iff_open(filename, &id, &len);
	if(f == NULL)
	{
		g_warning("can't open file '%s'", filename);
		return FALSE;
	}

	if((id != G3D_IFF_MKID('L','W','O','B')) &&
		(id != G3D_IFF_MKID('L','W','O','2')))
	{
		g_warning("file '%s' is not a LightWave object", filename);
		fclose(f);
		return FALSE;
	}

	/* LWO2 object file */
	if(id == G3D_IFF_MKID('L','W','O','2'))
		flags |= LW_F_LWO2;

	g3d_context_update_progress_bar(context, 0.0, TRUE);

	material = g3d_material_new();
	material->name = g_strdup("fallback material");
	model->materials = g_slist_append(model->materials, material);

	lwo_read_directory(f, len, model, lwo_object, id, 1, flags, context);

	fclose(f);

	g3d_context_update_progress_bar(context, 0.0, FALSE);

#if DEBUG > 2
	g_printerr("DEBUG: lwo file loaded\n");
#endif

	return TRUE;
}

gchar *plugin_description(G3DContext *context)
{
	return g_strdup(
		"Import plugin to load LightWave Objects (.lwo files)\n"
		"Author: Markus Dahms, heavily copied from gtkglarea examples\n");
}

gchar **plugin_extensions(G3DContext *context)
{
	return g_strsplit("lwo:lwb:lw", ":", 0);
}

/*****************************************************************************/
/* private                                                                   */
/*****************************************************************************/

static gint lwo_read_string(FILE *f, char *s);
static void lwo_read_srfs(FILE *f, gint nbytes, G3DObject *object);
static void lwo_read_surf(FILE *f, gint nbytes, G3DObject *object,
	guint32 flags);
static void lwo_read_pols(FILE *f, gint nbytes,
	G3DModel *model, G3DObject *object, guint32 flags, G3DContext *context);
static void lwo_read_pnts(FILE *f, gint nbytes, G3DObject *object,
	G3DContext *context);

static G3DObject *lwo_create_object(FILE *f, G3DModel *model, guint32 flags)
{
	G3DObject *object = g_new0(G3DObject, 1);
	object->name = g_strdup_printf("LWO%c object @ 0x%08lx",
		(flags & LW_F_LWO2) ? '2' : 'B', ftell(f) - 8);
	model->objects = g_slist_append(model->objects, object);

#if 0
	/* LWO files should have correct faces */
	model->glflags &= ~G3D_FLAG_GL_ALLTWOSIDE;
#endif

	return object;
}

static gboolean lwo_read_directory(FILE *f, guint32 nbytes, G3DModel *model,
	gpointer pobject, guint32 parentid, guint32 level, guint32 flags,
	G3DContext *context)
{
	G3DObject *object = NULL;
	int toread = nbytes;
	float pscale = (float)toread / 100.0;
	while(toread > 0)
	{
		guint32 id, len;

		if(level == 0)
		{
			g3d_context_update_progress_bar(context,
				100.0 - (float)toread / pscale, TRUE);
		}

		toread -= g3d_iff_readchunk(f, &id, &len);
#if DEBUG > 0
		g_printerr("%.*s[%c%c%c%c] @ 0x%08lx (%d bytes)\n",
			level * 2, "                   ",
			(id >> 24) & 0xFF, (id >> 16) & 0xFF,
			(id >> 8) & 0xFF, id & 0xFF,
			ftell(f) - 8, len);
#endif
		switch(id)
		{
			case G3D_IFF_MKID('P','N','T','S'):
				if(object == NULL)
					object = lwo_create_object(f, model, flags);
				lwo_read_pnts(f, len, object, context);
				break;

			case G3D_IFF_MKID('P','O','L','S'):
				lwo_read_pols(f, len, model, object, flags, context);
				break;

			case G3D_IFF_MKID('S','R','F','S'):
				/* LWOB: surfaces */
				if(object == NULL)
					object = lwo_create_object(f, model, flags);
				lwo_read_srfs(f, len, object);
				break;

			case G3D_IFF_MKID('S','U','R','F'):
				lwo_read_surf(f, len, object, flags);
				break;

			case G3D_IFF_MKID('T','A','G','S'):
				/* LWO2: first (used) chunk of object */
				object = lwo_create_object(f, model, flags);
				lwo_read_srfs(f, len, object);
				break;

			default:
				fseek(f, len + (len%2), SEEK_CUR);
				break;
		}
	}
	return TRUE;
}

/*****************************************************************************/
/* LWO specific                                                              */
/*****************************************************************************/

static gint lwo_read_string(FILE *f, char *s)
{
	gint c;
	gint cnt = 0;
	do {
		c = g3d_read_int8(f);
		if (cnt < LW_MAX_NAME_LEN)
		  s[cnt] = c;
		else
		  s[LW_MAX_NAME_LEN-1] = 0;
		cnt++;
	} while (c != 0);
	/* if length of string (including \0) is odd skip another byte */
	if (cnt%2) {
		g3d_read_int8(f);
		cnt++;
	}
	return cnt;
}

static guint32 lwo_read_vx(FILE *f, guint *index)
{
	*index = g3d_read_int16_be(f);
	if((*index & 0xFF00) == 0xFF00)
	{
		*index <<= 16;
		*index += g3d_read_int16_be(f);
		*index &= 0x00FFFFFF;
		return 4;
	}
	else
	{
		return 2;
	}
}

static void lwo_read_srfs(FILE *f, gint nbytes, G3DObject *object)
{
#if DEBUG > 2
	g_printerr("lwo_read_srfs {");
#endif
	while(nbytes > 0)
	{
		char text[1024];
		G3DMaterial *material = g3d_material_new();

		nbytes -= lwo_read_string(f, text);
		material->name = g_strdup(text);
		object->materials = g_slist_append(object->materials, material);
	}
#if DEBUG > 2
	g_printerr(" }\n");
#endif
}

static void lwo_read_surf(FILE *f, gint nbytes, G3DObject *object,
	guint32 flags)
{
	GSList *mlist;
	G3DMaterial *material = NULL, *tmat;
	gfloat tmpf;
	char name[LW_MAX_NAME_LEN];
	guint32 len = 0, tmp;

	len = lwo_read_string(f, name);
	nbytes -= len;

	if(flags & LW_F_LWO2)
	{
		g3d_read_int16_be(f);
		nbytes -= 2;
	}

	mlist = object->materials;
	g_return_if_fail(mlist != NULL);
	while(mlist != NULL)
	{
		tmat = (G3DMaterial*)mlist->data;
		if(strcmp(name, tmat->name) == 0)
		{
			material = tmat;
			break;
		}
		mlist = mlist->next;
	}
	g_return_if_fail(material != NULL);
#if DEBUG > 1
	g_print("LWO: surf: %s\n", material->name);
#endif

	material->a = 1.0;

	while(nbytes > 0)
	{
		gint32 id = g3d_read_int32_be(f);
		nbytes -= 4;
		len = g3d_read_int16_be(f);
		nbytes -= 2;

		switch(id)
		{
			case G3D_IFF_MKID('C','O','L','R'):
				if(flags & LW_F_LWO2)
				{
					material->r = g3d_read_float_be(f);
					material->g = g3d_read_float_be(f);
					material->b = g3d_read_float_be(f);
#if DEBUG > 0
					g_print("LWO: SURF: COLR: %+1.2f %+1.2f %+1.2f\n",
						material->r, material->g, material->b);
#endif
					g3d_read_int16_be(f);
					nbytes -= 14;
				}
				else
				{
					material->r = g3d_read_int8(f) / 255.0;
					material->g = g3d_read_int8(f) / 255.0;
					material->b = g3d_read_int8(f) / 255.0;
					g3d_read_int8(f);
					nbytes -= 4;
				}
				break;

			case G3D_IFF_MKID('D','I','F','F'):
				/* diffuse */
				if(flags & LW_F_LWO2)
				{
					g3d_read_float_be(f); /* intensity */
					nbytes -= 4;
					nbytes -= lwo_read_vx(f, &tmp); /* envelope ?? */
				}
				else
				{
					g3d_read_int16_be(f);
					nbytes -= 2;
				}
				break;

			case G3D_IFF_MKID('S','P','E','C'):
				/* specular */
				if(flags & LW_F_LWO2)
				{
					tmpf = 1.0 - g3d_read_float_be(f); /* intensity */
					nbytes -= 4;
					nbytes -= lwo_read_vx(f, &tmp); /* envelope ?? */
				}
				else
				{
					tmpf = 1.0 - (float)g3d_read_int16_be(f) / 256.0;
					nbytes -= 2;
				}

				material->specular[0] = material->r * tmpf;
				material->specular[1] = material->g * tmpf;
				material->specular[2] = material->b * tmpf;
				break;

			case G3D_IFF_MKID('T','R','A','N'):
				/* transparency */
				if(flags & LW_F_LWO2)
				{
					material->a = 1.0 - g3d_read_float_be(f); /* intensity */
					nbytes -= 4;
					nbytes -= lwo_read_vx(f, &tmp); /* envelope ?? */
				}
				else
				{
					material->a = 1.0 - (float)g3d_read_int16_be(f) / 256.0;
					nbytes -= 2;
				}
				if(material->a < 0.1)
					material->a = 0.1;
				break;

			default:
#if DEBUG > 0
				g_print("LWO: surf: [%c%c%c%c] (%d bytes)\n",
					(id >> 24) & 0xFF, (id >> 16) & 0xFF,
					(id >> 8) & 0xFF, id & 0xFF,
					len);
#endif
				fseek(f, len+(len%2), SEEK_CUR);
				nbytes -= len+(len%2);
				break;
		}
	}
#if DEBUG > 2
	g_printerr(" }\n");
#endif
}

static void lwo_read_pols(FILE *f, gint nbytes,
	G3DModel *model, G3DObject *object, guint32 flags, G3DContext *context)
{
	G3DFace *face;
	guint32 n = 0, type, i;
	gint16 nmat;

	if(flags & LW_F_LWO2)
	{
		type = g3d_read_int32_be(f);
		nbytes -= 4;

		if(type != G3D_IFF_MKID('F', 'A', 'C', 'E'))
		{
			fseek(f, nbytes, SEEK_CUR);
			return;
		}
	}

	while(nbytes > 0)
	{
		n++;
		face = g_new0(G3DFace, 1);
		face->vertex_count = g3d_read_int16_be(f);
		nbytes -= 2;

		if(flags & LW_F_LWO2)
			face->vertex_count &= 0x03FF; /* 6 high order bits are flags */

		face->vertex_indices = g_new0(guint32, face->vertex_count);

		for(i = 0; i < face->vertex_count; i ++)
		{
			if(flags & LW_F_LWO2)
			{
				nbytes -= lwo_read_vx(f, &(face->vertex_indices[i]));
#if DEBUG > 0
				if(face->vertex_indices[i] >= object->vertex_count)
				{
					g_print(
						"LWO: vertex_indices[%d] (%d) >= vertex_count (%d)\n",
						i, face->vertex_indices[i], object->vertex_count);
				}
#endif
			}
			else
			{
				face->vertex_indices[i] = g3d_read_int16_be(f);
				nbytes -= 2;

				if(face->vertex_indices[i] > object->vertex_count)
				{
#if DEBUG > 0
					g_print(
						"LWO: vertex_indices[%d] (%d) >= vertex_count (%d)\n",
						i, face->vertex_indices[i], object->vertex_count);
#endif
					face->vertex_indices[i] = 0;
				}
			}
			g3d_context_update_interface(context);
		}

		if(!(flags & LW_F_LWO2))
		{
			nmat = g3d_read_int16_be(f);
			nbytes -= 2;

			if(nmat < 0)
			{
				/* detail polygons, skipped */
				int det_cnt = g3d_read_int16_be(f);
				nbytes -= 2;
				nmat *= -1;
				while(det_cnt-- > 0)
				{
					int cnt = g3d_read_int16_be(f);
					nbytes -= 2;

					fseek(f, cnt*2+2, SEEK_CUR);
					nbytes -= cnt*2+2;
					g3d_context_update_interface(context);
				}
			}
			else if(nmat == 0)
			{
				nmat = 1;
			}

			face->material = g_slist_nth_data(object->materials,
				(guint) nmat - 1);
			if(face->material == NULL)
			{
#if DEBUG > 0
				g_print("LWO: face->material is NULL (#%d)\n", nmat - 1);
#endif
				face->material = g_slist_nth_data(model->materials, 0);
			}
#if DEBUG > 3
			g_printerr("lwo_read_pols: face #%d (nbytes: %d)\n", n, nb);
#endif
		} /* ! LW_F_LWO2 */
		else
		{
			face->material = g_slist_nth_data(object->materials, 0);
			if(face->material == NULL)
				face->material = g_slist_nth_data(model->materials, 0);
		}

		if(face->vertex_count < 3)
		{
			g_free(face->vertex_indices);
			g_free(face);
		}
		else
			object->faces = g_slist_prepend(object->faces, face);
	} /* nb > 0 */

#if DEBUG > 0
	g_print("LWO: POLS: %d faces\n", g_slist_length(object->faces));
#endif

	if(nbytes != 0)
	{
		fseek(f, nbytes, SEEK_CUR);
#if DEBUG > 0
		g_print("LWO: POLS: had to seek %d bytes", nbytes);
#endif
	}
}

static void lwo_read_pnts(FILE *f, gint nbytes, G3DObject *object,
	G3DContext *context)
{
	guint32 i, off;

	off = object->vertex_count;
	object->vertex_count += (nbytes / 12);
	if(object->vertex_count < 3)
	{
#if DEBUG > 0
		g_print("LWO: PNTS: object->vertex_count < 3 (%d)\n",
			object->vertex_count);
#endif
		return;
	}

	object->vertex_data = g_realloc(object->vertex_data,
		sizeof(gfloat) * object->vertex_count * 3);

	for(i = off; i < object->vertex_count; i ++)
	{
		object->vertex_data[i * 3 + 0] = g3d_read_float_be(f);
		object->vertex_data[i * 3 + 1] = g3d_read_float_be(f);
		object->vertex_data[i * 3 + 2] = g3d_read_float_be(f);
		g3d_context_update_interface(context);
	}
}


