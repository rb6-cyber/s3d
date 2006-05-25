/* $Id: imp_dxf.c,v 1.1.2.2 2006/01/23 17:03:06 dahms Exp $ */

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
#include <locale.h>

#include <g3d/types.h>
#include <g3d/material.h>
#include <g3d/read.h>

int dxf_read_section(FILE *f, G3DModel *model, G3DObject *object, int bin);
gint dxf_read_code(FILE *f, int binary);
char *dxf_read_string(FILE *f, char *value, int binary);
gdouble dxf_read_float64(FILE *f, int binary);
int dxf_skip_section(FILE *f, int binary);

gboolean plugin_load_model(G3DContext *context, const gchar *filename,
	G3DModel *model, gpointer user_data)
{
	FILE *f;
	char binmagic[22];
	int binarydxf = 0;
	G3DObject *object;
	G3DMaterial *material;

	setlocale(LC_NUMERIC, "C");

	f = fopen(filename, "r");
	if(f == NULL)
	{
		g_warning("can't open file '%s'", filename);
		return FALSE;
	}

	if((fread(binmagic, 1, 22, f) == 22) &&
		 (strncmp(binmagic, "AutoCAD Binary DXF", 18) == 0)) binarydxf = 1;
	else rewind(f);

	object = g_new0(G3DObject, 1);
	object->name = g_strdup("DXF Object");
	model->objects = g_slist_append(model->objects, object);

	material = g3d_material_new();
	material->name = g_strdup("default material");
	material->flags |= G3D_FLAG_MAT_TWOSIDE;
	object->materials = g_slist_append(object->materials, material);

	while(!feof(f))
	{
		int retval = dxf_read_section(f, model, object, binarydxf);
		if(retval != TRUE)
		{
			fclose(f);
			if(retval == 0xE0F) return TRUE;
			g_printerr("error in section..\n");
			return FALSE;
		}
	}

	fclose(f);
	return TRUE;
}

char *plugin_description(void)
{
	return g_strdup(
		"Import plugin for AutoCAD DXF files\n");
}

char **plugin_extensions(void)
{
	return g_strsplit("dxf", ":", 0);
}

/*****************************************************************************/

int dxf_read_section(FILE *f, G3DModel *model, G3DObject *object, int bin)
{
	gint grpcode;
	G3DFace *face = NULL;
	char val_str[256]; /* length of line, 255 should be enough */

	grpcode = dxf_read_code(f, bin);
	if(grpcode != 0) 
	{
#if DEBUG > 0
		g_printerr("unexpected group code: %d (0 expected)\n", grpcode);
#endif
		return FALSE;
	}
	dxf_read_string(f, val_str, bin);
	if(strcmp("EOF", val_str) == 0) return 0xE0F;
	if(strcmp("SECTION", val_str) != 0) 
	{
#if DEBUG > 0
		g_printerr("SECTION expected, found: %s\n", val_str);
#endif
		return FALSE;
	}
	grpcode = dxf_read_code(f, bin);
	if(grpcode != 2) 
	{
#if DEBUG > 0
		g_printerr("unexpected group code: %d (2 expected)\n", grpcode);
#endif
		return FALSE;
	}
	dxf_read_string(f, val_str, bin);
	
	if((strcmp(val_str, "HEADER") == 0) ||
		 (strcmp(val_str, "CLASSES") == 0) ||
		 (strcmp(val_str, "TABLES") == 0) ||
		 (strcmp(val_str, "BLOCKS") == 0) ||
		 (strcmp(val_str, "OBJECTS") == 0))
	{
#if DEBUG > 0
		g_printerr("skipping section: %s\n", val_str);
#endif
		dxf_skip_section(f, bin);
	}
	else if(strcmp(val_str, "ENTITIES") == 0)
	{
		int key;
		char str[128];
		gdouble val_f64;
#if DEBUG > 0
		g_printerr("processing entities section...\n");
#endif
		while(1)
		{
			key = dxf_read_code(f, bin);
			switch(key)
			{
				case -1:
					return 0xE0F;
					break;
				case 0:
					face = NULL;
					dxf_read_string(f, str, bin);
					if(strcmp(str, "ENDSEC") == 0) return TRUE;
					else if(strcmp("3DFACE", str) == 0)
					{
						int nfaces, i;

						face = g_new0(G3DFace, 1);
						object->faces = g_slist_prepend(object->faces, face);
						nfaces = g_slist_length(object->faces);
#if DEBUG > 2
						g_printerr("creating face... (#%d)\n", nfaces);
#endif
						object->vertex_count = nfaces * 4;
						object->vertex_data = g_realloc(object->vertex_data,
							object->vertex_count * 3 * sizeof(gfloat));

						face->vertex_count = 4;
						face->vertex_indices = g_new0(guint32, 4);
						for(i = 0; i < 4; i ++)
						{
							face->vertex_indices[i] = (nfaces-1) * 4 + i;
							object->vertex_data[
								face->vertex_indices[i] * 3 + 0] = 0.0;
							object->vertex_data[
								face->vertex_indices[i] * 3 + 1] = 0.0;
							object->vertex_data[
								face->vertex_indices[i] * 3 + 2] = 0.0;
						}
						face->material =
							g_slist_nth_data(object->materials, 0);
					}
					break;

				case 8:
					dxf_read_string(f, str, bin);
					break;

				case 10: case 20: case 30:
				case 11: case 21: case 31:
				case 12: case 22: case 32:
				case 13: case 23: case 33:
					val_f64 = dxf_read_float64(f, bin);
					if(face != NULL)
					{
						object->vertex_data[
							face->vertex_indices[key % 10] * 3 +
							(key / 10) - 1] =
							(gfloat)val_f64;
					}
					break;

				case 50:
				case 210: case 220: case 230:
					dxf_read_float64(f, bin);
					break;

				default:
#if DEBUG > 0
					g_printerr("unhandled key: %d, try to skip... "
						"(pos in file: %ld)\n", key, ftell(f));
#endif
					if(bin)
						return FALSE; /* unable to skip unknown type */
					else
						fgets(str, 128, f);
			}
		}
#if 0
		dxf_skip_section(f, bin);
#endif
	}
	else
	{
#if DEBUG > 0
		g_printerr("unknown section '%s', skipping...\n", val_str);
#endif
		dxf_skip_section(f, bin);
	}
	return TRUE;
}

char *dxf_read_string(FILE *f, char *value, int binary)
{
	if(binary)
	{
		int pos = 0;
		int c;
		do
		{
			c = fgetc(f);
			value[pos] = (char)c;
			pos++;
		} while(c != '\0');
		return value;
	}
	else
	{
		char line[128];

		fgets(line, 128, f);
		if(sscanf(line, "%s", value) == 1)
			return g_strchomp(value);
		if(sscanf(line, " %s", value) == 1)
			return g_strchomp(value);
		return NULL;
	}
}

gint dxf_read_code(FILE *f, int binary)
{
	if(binary)
	{
		return g3d_read_int8(f);
	}
	else
	{
		int val;
		char line[256];

		fgets(line, 256, f);
		if(sscanf(line, "%d", &val) == 1)
			return val;
		if(sscanf(line, " %d", &val) == 1)
			return val;
		else return -1;
	}
}

gdouble dxf_read_float64(FILE *f, int binary)
{
	if(binary)
	{
		gint64 i64 = (gint64)g3d_read_int32_le(f) |
			((gint64)g3d_read_int32_le(f) << 32);
		return (gdouble)i64;
	}
	else
	{
		gdouble val;
		char line[256];

		fgets(line, 256, f);
		if(sscanf(line, "%lf", &val) == 1)
			return val;
		if(sscanf(line, " %lf", &val) == 1)
			return val;
		else
			return 0.0;
	}
}

int dxf_skip_section(FILE *f, int binary)
{
	if(binary)
	{
		while(!feof(f))
		{
			int c, read;
			char buf[7];
			do { c = fgetc(f); } while(c != 0);
			read = fread(buf, 1, 7, f);
			if((read == 7) && (strncmp(buf, "ENDSEC", 6) == 0)) return TRUE;
			else fseek(f, -read, SEEK_CUR);
		}
	}
	else
	{
		while(!feof(f))
		{
			char line[128];
			
			fgets(line, 128, f);
			if(strncmp(line, "ENDSEC", 6) == 0) return TRUE;
		}
	}
	return TRUE;
}

