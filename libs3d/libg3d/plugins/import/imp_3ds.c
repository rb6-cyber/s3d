/* $Id: imp_3ds.c,v 1.1.2.3 2006/01/23 17:03:05 dahms Exp $ */

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
#include <string.h>
#include <stdarg.h>

#include <g3d/types.h>
#include <g3d/read.h>
#include <g3d/material.h>
#include <g3d/texture.h>

#include "imp_3ds.h"
#include "imp_3ds_chunks.h"

/*****************************************************************************/
/* plugin interface                                                          */
/*****************************************************************************/

gboolean plugin_load_model(G3DContext *context, const gchar *filename,
	G3DModel *model, gpointer plugin_data)
{
	FILE *f;
	gint32 nbytes, magic;
	gboolean retval;
	x3ds_global_data global;
	x3ds_parent_data *parent;
	long int fpos;

	f = fopen(filename, "r");
	if(f == NULL)
	{
		g_warning("can't open file '%s'", filename);
		return FALSE;
	}

	magic = g3d_read_int16_le(f);
	if((magic != 0x4D4D) && (magic != 0xC23D))
	{
		g_warning("file %s is not a 3ds file", filename);
		fclose(f);
		return FALSE;
	}
	nbytes = g3d_read_int32_le(f);
	nbytes -= 6;
	g_printerr("[%4.4X] 3DS file: main length: %d\n", magic, nbytes);

	global.context = context;
	global.model = model;
	global.f = f;
	global.scale = 1.0;
	global.max_tex_id = 0;

	/* get size of file */
	fpos = ftell(global.f);
	fseek(global.f, 0, SEEK_END);
	global.max_fpos = ftell(global.f);
	fseek(global.f, fpos, SEEK_SET);

	parent = g_new0(x3ds_parent_data, 1);
	parent->id = magic;
	parent->nb = nbytes;

	retval = x3ds_read_ctnr(&global, parent);

	g_free(parent);

	fclose(f);

#if DEBUG > 0
	if(retval)
		g_printerr("imp_3ds.c: %s successfully loaded\n", filename);
#endif

	return retval;
}

gchar *plugin_description(void)
{
	return g_strdup(
		"Import plugin for AutoCAD 3D Studio files\n");
}

gchar **plugin_extensions(void)
{
	return g_strsplit("3ds:prj", ":", 0);
}

/*****************************************************************************/

gboolean x3ds_read_ctnr(x3ds_global_data *global, x3ds_parent_data *parent)
{
	gint32 chunk_id, chunk_len, i;
	x3ds_parent_data *subparent;
	gpointer level_object;

	level_object = NULL;

	while(parent->nb > 0)
	{
		chunk_id  = g3d_read_int16_le(global->f);
		chunk_len = g3d_read_int32_le(global->f);
		parent->nb -= 6;
		chunk_len -= 6;

		i = 0;
		while((x3ds_chunks[i].id != 0) && (x3ds_chunks[i].id != chunk_id))
			i ++;

		if(x3ds_chunks[i].id == chunk_id)
		{
			x3ds_debug(parent->level, "[0x%04X][%c%c] %s (%d bytes)\n",
				chunk_id,
				x3ds_chunks[i].container ? 'c' : ' ',
				x3ds_chunks[i].callback ? 'f' : ' ',
				x3ds_chunks[i].desc, chunk_len);

			if (chunk_id==0)
			{
				g_printerr("error: bad chunk id\n");
				return FALSE;
			}

			subparent = g_new0(x3ds_parent_data, 1);
			subparent->id = parent->id;
			subparent->object = parent->object;
			subparent->level = parent->level + 1;
			subparent->level_object = level_object;
			subparent->nb = chunk_len;

			if(x3ds_chunks[i].callback)
			{
				/* callback may change "nb" and "object" of
				 * "subparent" structure for following container run */

				x3ds_chunks[i].callback(global, subparent);
			}

			subparent->id = chunk_id;

			if(x3ds_chunks[i].container)
			{
				if(x3ds_read_ctnr(global, subparent) == FALSE)
				{
					/* abort on error */
					return FALSE;
				}
			}

			if(subparent->nb)
			{
				fseek(global->f, subparent->nb, SEEK_CUR);
			}

			level_object = subparent->level_object;

			g_free(subparent);
		}
		else
		{
			g_printerr("[3DS] unknown chunk type 0x%04X\n", chunk_id);
			fseek(global->f, chunk_len, SEEK_CUR);
		}

		parent->nb -= chunk_len;

		/* update progress bar */
		x3ds_update_progress(global);
	}

	return TRUE;
}

void x3ds_update_progress(x3ds_global_data *global)
{
	long int fpos;

	/* update progress bar */
	fpos = ftell(global->f);
	g3d_context_update_progress_bar(global->context,
		((gfloat)fpos / (gfloat)global->max_fpos), TRUE);
}

gint32 x3ds_read_cstr(FILE *f, char *string)
{
	gint32 n = 0;
	char c;
	do
	{
		c = g3d_read_int8(f);
		string[n] = c;
		n++;
	} while(c != 0);
	return n;
}

void x3ds_debug(int level, char *format, ...)
{
#if DEBUG > 0
	int i;
	va_list parms;

	for(i=0; i<level; i++) fprintf(stderr, "  ");
	va_start(parms, format);
	vfprintf(stderr, format, parms);
	va_end(parms);
#endif
}

G3DObject *x3ds_newobject(G3DModel *model, const char *name)
{
	G3DObject *object = g_malloc0(sizeof(G3DObject));
	G3DMaterial *material = g3d_material_new();

	object->name = g_strdup(name);
	object->faces = NULL;
	model->objects = g_slist_append(model->objects, object);
	object->materials = g_slist_append(object->materials, material);

	return object;
}

