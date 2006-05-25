/* $Id: imp_cob.c,v 1.1.2.3 2006/01/23 17:03:06 dahms Exp $ */

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <g3d/types.h>
#include <g3d/context.h>
#include <g3d/material.h>
#include <g3d/vector.h>
#include <g3d/read.h>
#include <g3d/iff.h>

static gboolean cob_read_file_bin(FILE *f, G3DModel *model, gboolean is_be,
	G3DContext *context);

gboolean plugin_load_model(G3DContext *context, const gchar *filename,
	G3DModel *model, gpointer user_data)
{
	FILE *f;
	gchar header[32];
	gboolean file_is_ascii, file_is_be;

	f = fopen(filename, "rb");
	if(f == NULL)
	{
		g_warning("COB: could not open '%s'", filename);
		return FALSE;
	}

	if(fread(header, 1, 32, f) != 32)
	{
		g_warning("COB: could not read header");
		fclose(f);
		return FALSE;
	}

	if(strncmp(header, "Caligari ", 9) != 0)
	{
		g_warning("COB: '%s' is not a valid TrueSpace file", filename);
		fclose(f);
		return FALSE;
	}

	file_is_ascii = (header[15] == 'A');
	file_is_be = (header[16] == 'H');

#if DEBUG > 0
	g_print("COB: file version %.*s, %s, %s endian\n",
		6, header + 9,
		file_is_ascii ? "ASCII" : "binary",
		file_is_be ? "big" : "little" );
#endif

	if(file_is_ascii)
	{
		g_warning("COB: ASCII files are unsupported at the moment");
		fclose(f);
		return FALSE;
	}

	cob_read_file_bin(f, model, file_is_be, context);

	fclose(f);

	return TRUE;
}

char *plugin_description(void)
{
	return g_strdup("import plugin for Caligari TrueSpace objects\n");
}

char **plugin_extensions(void)
{
	return g_strsplit("cob", ":", 0);
}

/*
 * COB specific
 */

#define cob_read_e(file, what, be) \
	((be) ? g3d_read_ ## what ## _be(file) : \
		g3d_read_ ## what ## _le(file))

#define COB_F_HOLE     0x08
#define COB_F_BACKCULL 0x10

static gboolean cob_read_chunk_header_bin(FILE *f, gboolean is_be,
	guint32 *type, guint16 *ver_maj, guint16 *ver_min,
	guint32 *id, guint32 *parent_id, guint32 *len)
{
	*type =
		(g3d_read_int8(f) << 24) +
		(g3d_read_int8(f) << 16) +
		(g3d_read_int8(f) << 8) +
		(g3d_read_int8(f));
	*ver_maj = cob_read_e(f, int16, is_be);
	*ver_min = cob_read_e(f, int16, is_be);
	*id = cob_read_e(f, int32, is_be);
	*parent_id = cob_read_e(f, int32, is_be);
	*len = cob_read_e(f, int32, is_be);

	return TRUE;
}

static gchar *cob_read_name_bin(FILE *f, guint32 *len, gboolean is_be)
{
	gchar *buffer, *name;
	guint32 dc, namelen;

	dc = cob_read_e(f, int16, is_be);
	*len -= 2;

	namelen = cob_read_e(f, int16, is_be);
	*len -= 2;

	buffer = g_new0(gchar, namelen + 1);
	fread(buffer, 1, namelen, f);
	*len -= namelen;

	name = g_strdup_printf("%s (%d)", buffer, dc);

	g_free(buffer);
	return name;
}

static G3DObject *cob_read_grou_bin(FILE *f, guint32 len, gboolean is_be)
{
	G3DObject *object;

#if DEBUG > 0
	g_print("COB: Grou chunk @ 0x%08lx\n", ftell(f));
#endif

	object = g_new0(G3DObject, 1);
	object->name = cob_read_name_bin(f, &len, is_be);
#if DEBUG > 0
	g_print("COB: Grou: name is '%s'\n", object->name);
#endif

	if(len > 0)
		fseek(f, len, SEEK_CUR);

	return object;
}

static G3DObject *cob_read_polh_bin(FILE *f, guint32 len, gboolean is_be,
	G3DContext *context)
{
	G3DObject *object;
	guint32 nfaces, i;
	gfloat curpos[16];

#if DEBUG > 0
	g_print("COB: PolH chunk @ 0x%08lx\n", ftell(f));
#endif

	object = g_new0(G3DObject, 1);
	object->name = cob_read_name_bin(f, &len, is_be);
#if DEBUG > 0
	g_print("COB: PolH: name is '%s'\n", object->name);
#endif

	/* local axes: 4 x 12 */
	fseek(f, 48, SEEK_CUR);
	len -= 48;

	/* current position: 3 x 16 */
	curpos[12] = curpos[13] = curpos[14] = 0.0;
	curpos[15] = 1.0;

	for(i = 0; i < 12; i ++)
	{
		curpos[i] = cob_read_e(f, float, is_be);
		len -= 4;
	}

#if DEBUG > 0
	for(i = 0; i < 4; i ++)
	{
		g_print("COB: PolH: CurPos: %+1.2f %+1.2f %+1.2f %+1.2f\n",
			curpos[i * 4 + 0], curpos[i * 4 + 1],
			curpos[i * 4 + 2], curpos[i * 4 + 3]);
	}
#endif

	/* vertex list */
	object->vertex_count = cob_read_e(f, int32, is_be);
	len -= 4;
	object->vertex_data = g_new0(gfloat, object->vertex_count * 3);
	for(i = 0; i < object->vertex_count; i ++)
	{
		object->vertex_data[i*3+0] = cob_read_e(f, float, is_be);
		object->vertex_data[i*3+1] = cob_read_e(f, float, is_be);
		object->vertex_data[i*3+2] = cob_read_e(f, float, is_be);
		g3d_vector_transform(
			&(object->vertex_data[i*3+0]),
			&(object->vertex_data[i*3+1]),
			&(object->vertex_data[i*3+2]),
			curpos);
		g3d_context_update_interface(context);
		len -= 12;
	}

	/* texture vertex list */
	object->tex_vertex_count = cob_read_e(f, int32, is_be);
	len -= 4;
	object->tex_vertex_data = g_new0(gfloat, object->tex_vertex_count * 2);
	for(i = 0; i < object->tex_vertex_count; i ++)
	{
		object->tex_vertex_data[i*2+0] = cob_read_e(f, float, is_be);
		object->tex_vertex_data[i*2+1] = cob_read_e(f, float, is_be);
		g3d_context_update_interface(context);
		len -= 8;
	}

	/* faces and holes */
	nfaces = cob_read_e(f, int32, is_be);
	len -= 4;
	for(i = 0; i < nfaces; i ++)
	{
		G3DFace *face;
		guint8 flags;
		guint32 nverts, j, matidx;

		flags = g3d_read_int8(f);
		len -= 1;
		if(flags & COB_F_HOLE)
		{
			/* hole: ignore for now */
			nverts = cob_read_e(f, int16, is_be);
			len -= 2;
			for(j = 0; j < nverts; j ++)
			{
				cob_read_e(f, int32, is_be);
				cob_read_e(f, int32, is_be);
				len -= 8;
			}
		}
		else
		{
			/* "real" face */
			nverts = cob_read_e(f, int16, is_be);
			len -= 2;
			matidx = cob_read_e(f, int16, is_be);
			len -= 2;
			face = g_new0(G3DFace, 1);

			face->vertex_count = nverts;
			face->vertex_indices = g_new0(guint32, nverts);
			for(j = 0; j < nverts; j ++)
			{
				face->vertex_indices[j] = cob_read_e(f, int32, is_be);
				cob_read_e(f, int32, is_be); /* UV indices */
				len -= 8;
			}

			/* set material */
			if(matidx < g_slist_length(object->materials))
				face->material = g_slist_nth_data(object->materials, matidx);
			else
			{
				for(j = g_slist_length(object->materials); j <= matidx; j ++)
				{
					G3DMaterial *material = g3d_material_new();
					material->name = g_strdup_printf("fallback material %d",
						j);
					object->materials =
						g_slist_append(object->materials, material);
				}

				face->material = g_slist_nth_data(object->materials, matidx);
			}

			object->faces = g_slist_prepend(object->faces, face);
		}

		g3d_context_update_interface(context);
	}

	if(len > 0)
		fseek(f, len, SEEK_CUR);

	return object;
}

static int cob_read_mat1_bin(FILE *f, guint32 len, gboolean is_be,
	G3DObject *object)
{
	G3DMaterial *material;
	guint32 matidx;

	g_return_val_if_fail(object != NULL, FALSE);

	matidx = cob_read_e(f, int16, is_be);
	len -= 2;

	material = g_slist_nth_data(object->materials, matidx);

	if(material)
	{
		g_free(material->name);
		material->name = g_strdup_printf("material #%d", matidx);

		/* shader type */
		g3d_read_int8(f);
		len --;

		/* facet type */
		g3d_read_int8(f);
		len --;

		/* autofacet angle */
		g3d_read_int8(f);
		len --;

		/* RGBA */
		material->r = cob_read_e(f, float, is_be);
		material->g = cob_read_e(f, float, is_be);
		material->b = cob_read_e(f, float, is_be);
		material->a = cob_read_e(f, float, is_be);
		len -= 16;
	}
	else
	{
#if DEBUG > 0
		g_print("COB: Mat1: material #%d not used, ignoring...\n", matidx);
#endif
	}

    if(len > 0)
        fseek(f, len, SEEK_CUR);

	return TRUE;
}

static int cob_read_unit_bin(FILE *f, guint32 len, gboolean is_be)
{
	guint16 uidx;
	static gchar *units[] = {
		"millimeters",
		"centimeters",
		"meters",
		"kilometers",
		"inches",
		"feet",
		"yards",
		"miles",
		"points" };

	uidx = cob_read_e(f, int16, is_be);
#if DEBUG > 0
	if(uidx >= (sizeof(units) / sizeof(gchar *)))
	{
		g_print("COB: Unit: out of range (%d)\n", uidx);
		return FALSE;
	}
	g_print("COB: units are %s\n", units[uidx]);
#endif
	return TRUE;
}

static gboolean cob_read_file_bin(FILE *f, G3DModel *model, gboolean is_be,
	G3DContext *context)
{
	G3DObject *object = NULL;
	guint32 type, id, parent_id, len;
	guint16 ver_min, ver_maj;
	gboolean exit = FALSE;

	do
	{
		cob_read_chunk_header_bin(f, is_be, &type, &ver_maj, &ver_min,
			&id, &parent_id, &len);

		switch(type)
		{
			case G3D_IFF_MKID('E', 'N', 'D', ' '):
				/* end of file */
				exit = TRUE;
				break;

			case G3D_IFF_MKID('G', 'r', 'o', 'u'):
				/* group */
				object = cob_read_grou_bin(f, len, is_be);
#if 0
				model->objects = g_slist_append(model->objects, object);
#endif
				break;

			case G3D_IFF_MKID('M', 'a', 't', '1'):
				cob_read_mat1_bin(f, len, is_be, object);
				break;

			case G3D_IFF_MKID('P', 'o', 'l', 'H'):
				/* polygonal data */
				object = cob_read_polh_bin(f, len, is_be, context);
				model->objects = g_slist_append(model->objects, object);
				break;

			case G3D_IFF_MKID('U', 'n', 'i', 't'):
				cob_read_unit_bin(f, len, is_be);
				break;

			default:
#if DEBUG > 0
				g_print("COB: unknown chunk type: %c%c%c%c: 0x%08x (0x%08x),"
					" %u bytes\n",
					(type >> 24) & 0xFF, (type >> 16) & 0xFF,
					(type >> 8) & 0xFF, type & 0xFF,
					id, parent_id, len);
#endif
				fseek(f, len, SEEK_CUR);
				break;
		}
	}
	while(type && !exit);

	return TRUE;
}

#undef cob_read_e
