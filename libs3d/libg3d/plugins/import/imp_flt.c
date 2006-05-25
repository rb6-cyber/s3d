/* $Id: imp_flt.c,v 1.1.2.2 2006/01/23 17:03:06 dahms Exp $ */

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

#include <g3d/types.h>
#include <g3d/read.h>
#include <g3d/material.h>

gboolean flt_read_color_palette(FILE *f, gint32 len, G3DModel *model);
gboolean flt_read_texture_palette(FILE *f, gint32 len, G3DModel *model);
gboolean flt_read_material_palette(FILE *f, gint32 len, G3DModel *model);
gboolean flt_read_vertex_palette(FILE *f, gint32 len, G3DModel *model);
gboolean flt_read_light_source_palette(FILE *f, gint32 len, G3DModel *model);

gboolean plugin_load_model(G3DContext *context, const gchar *filename,
	G3DModel *model, gpointer user_data)
{
	FILE *f;
	guint16 opcode, rlen;

	f = fopen(filename, "rb");
	if(f == NULL)
	{
		g_warning("FLT: failed to open '%s'", filename);
		return FALSE;
	}

	while(!feof(f))
	{
		opcode = g3d_read_int16_be(f);
		rlen = g3d_read_int16_be(f);

		switch(opcode)
		{
			case 0:
				break;

			case 32: /* color palette record */
				flt_read_color_palette(f, rlen - 4, model);
				break;

			case 64: /* texture palette record */
				flt_read_texture_palette(f, rlen - 4, model);
				break;

			case 67: /* vertex palette record */
				flt_read_vertex_palette(f, rlen - 4, model);
				break;

			case 102: /* light source palette record */
				flt_read_light_source_palette(f, rlen - 4, model);
				break;

			case 113: /* material palette record */
				flt_read_material_palette(f, rlen - 4, model);
				break;

			default:
				g_print("FLT: op 0x%04x (%u, %u bytes)\n",
					opcode, opcode, rlen);
				if(rlen > 4)
					fseek(f, rlen - 4, SEEK_CUR);
				break;
		}
	}

	fclose(f);

	return TRUE;
}

gchar *plugin_description(G3DContext *context)
{
	return g_strdup("import plugin for OpenFlight files\n");
}

gchar **plugin_extensions(G3DContext *context)
{
	return g_strsplit("flt", ":", 0);
}

/*
 * FLT specific
 */

gboolean flt_read_color_palette(FILE *f, gint32 len, G3DModel *model)
{
#if 0
	guint32 i;
	G3DMaterial *material;

	for(i = 0; i < 1024; i ++)
	{
		material = g3d_material_new();

		material->a = g3d_read_int8(f) / 255.0;
		material->r = g3d_read_int8(f) / 255.0;
		material->g = g3d_read_int8(f) / 255.0;
		material->b = g3d_read_int8(f) / 255.0;
		material->name = g_strdup_printf(
			"color #%u: %+1.2f,%+1.2f,%+1.2f,%+1.2f",
			i, material->r, material->g, material->b, material->a);

		len -= 4;

		model->materials = g_slist_append(model->materials, material);
	}
#endif

	/* TODO: color names */

	if(len > 0)
		fseek(f, len, SEEK_CUR);

	return TRUE;
}

gboolean flt_read_texture_palette(FILE *f, gint32 len, G3DModel *model)
{
	gchar filename[200];
	guint32 pi, locx, locy;

	fread(filename, 1, 200, f);
	len -= 200;

	pi = g3d_read_int32_be(f);
	locx = g3d_read_int32_be(f);
	locy = g3d_read_int32_be(f);
	len -= 12;

	g_print("FLT: texture file '%s' (%u @ %u,%u)\n", filename, pi, locx, locy);

	if(len > 0)
		fseek(f, len, SEEK_CUR);

	return TRUE;
}

gboolean flt_read_material_palette(FILE *f, gint32 len, G3DModel *model)
{
	guint32 index, flags;
	gchar name[12];
	G3DMaterial *material;

	index = g3d_read_int32_be(f);
	len -= 4;
	if(index != g_slist_length(model->materials))
	{
		g_print("FLT: index (%d) != g_slist_length(model->materials)\n",
			index);
	}

	material = g3d_material_new();
	fread(name, 1, 12, f);
	len -= 12;
	material->name = g_strndup(name, 12);
	flags = g3d_read_int32_be(f);
	len -= 4;

	/* ambient */
	material->r = g3d_read_float_be(f);
	material->g = g3d_read_float_be(f);
	material->b = g3d_read_float_be(f);
	len -= 12;
	/* diffuse */
	g3d_read_float_be(f);
	g3d_read_float_be(f);
	g3d_read_float_be(f);
	len -= 12;
	/* specular */
	material->specular[0] = g3d_read_float_be(f);
	material->specular[1] = g3d_read_float_be(f);
	material->specular[2] = g3d_read_float_be(f);
	len -= 12;
	/* emissive */
	g3d_read_float_be(f);
	g3d_read_float_be(f);
	g3d_read_float_be(f);
	len -= 12;
	/* shininess */
	/* FIXME: divide by 128.0? */
	material->shininess = g3d_read_float_be(f);
	len -= 4;
	/* alpha */
	material->a = g3d_read_float_be(f);
	len -= 4;

	model->materials = g_slist_append(model->materials, material);

	/* 4 bytes reserved */

	if(len > 0)
		fseek(f, len, SEEK_CUR);

	return TRUE;
}

gboolean flt_read_vertex_palette(FILE *f, gint32 len, G3DModel *model)
{
	guint32 vlen;

	vlen = g3d_read_int32_be(f);

	g_print("FLT: vertex palette length: %u\n", vlen);

	return TRUE;
}

gboolean flt_read_light_source_palette(FILE *f, gint32 len, G3DModel *model)
{
	/* TODO: */

	if(len > 0)
		fseek(f, len, SEEK_CUR);

	return TRUE;
}
