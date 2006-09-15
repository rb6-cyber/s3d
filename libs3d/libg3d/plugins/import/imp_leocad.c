/* $Id: imp_leocad.c,v 1.1.2.4 2006/01/23 17:03:06 dahms Exp $ */

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
#include <locale.h>

#include <math.h>

#ifndef M_PI
#	define M_PI 3.14159265358979323846
#endif

#include <g3d/types.h>
#include <g3d/context.h>
#include <g3d/read.h>
#include <g3d/vector.h>
#include <g3d/matrix.h>

#include "imp_leocad_library.h"

static int leocad_load_lcd(const gchar *filename, G3DModel *model,
	LeoCadLibrary *library, G3DContext *context);

gpointer plugin_init(G3DContext *context)
{
	LeoCadLibrary *library;
	const gchar *libdir;

	libdir = g_getenv("LEOCAD_LIB");
	if(libdir == NULL)
		libdir = "/usr/local/share/leocad";

	library = leocad_library_load(libdir);

	if(library == NULL)
	{
		g_warning("LeoCAD: failed to load library");
		return NULL;
	}

	return library;
}

void plugin_cleanup(gpointer user_data)
{
	LeoCadLibrary *library;

	library = (LeoCadLibrary *)user_data;

	if(library)
		leocad_library_free(library);
}

gboolean plugin_load_model(G3DContext *context, const gchar *filename,
	G3DModel *model, gpointer user_data)
{
	LeoCadLibrary *library;

	library = (LeoCadLibrary *)user_data;

	if(library == NULL)
	{
		g_warning("LeoCAD: library not loaded");
		return FALSE;
	}

	setlocale(LC_NUMERIC, "C");

	return leocad_load_lcd(filename, model, library, context);
}

gchar *plugin_description(G3DContext *context)
{
	return g_strdup(
		"Import plugin for LeoCAD files\n");
}

gchar **plugin_extensions(G3DContext *context)
{
	return g_strsplit("lcd", ":", 0);
}

/*
 * LeoCAD specific stuff
 */

static gboolean leocad_load_lcd_piece(FILE *f, G3DModel *model,
	LeoCadLibrary *library, gfloat lcdversion)
{
	guint32 i, j, nkeys;
	guint16 ktime;
	guint8 pver, ktype, color = 0, len8;
	gchar name[9];
	gfloat param[4], matrix[16], mloc[16];
	gfloat offx = 0.0, offy = 0.0, offz = 0.0;
	gfloat rotx = 0.0, roty = 0.0, rotz = 0.0;
	G3DObject *object;
	G3DMaterial *mat_change;
	G3DFace *face;
	GSList *fitem;
	gboolean valid_matrix = FALSE;

	g3d_matrix_identity(mloc);
	g3d_matrix_identity(matrix);

	mat_change = leocad_library_get_nth_material(library, 0x10);

	if(lcdversion > 0.4)
	{
		pver = g3d_read_int8(f);

		if(pver >= 9)
		{

		}
		else /* pver < 9 */
		{
			if(pver > 5)
			{
				nkeys = g3d_read_int32_le(f);
				for(i = 0; i < nkeys; i ++)
				{
					/* param */
					param[0] = g3d_read_float_le(f);
					param[1] = g3d_read_float_le(f);
					param[2] = g3d_read_float_le(f);
					param[3] = g3d_read_float_le(f);

					/* time */
					ktime = g3d_read_int16_le(f);

					/* type */
					ktype = g3d_read_int8(f);

					/* get first frame */
					if(ktime == 1)
					{
						switch(ktype)
						{
							case 0x00: /* translation */
#if 1
								mloc[0 * 4 + 3] = param[0];
								mloc[1 * 4 + 3] = param[1];
								mloc[2 * 4 + 3] = param[2];
#else
								mloc[3 * 4 + 0] = param[0];
								mloc[3 * 4 + 1] = param[1];
								mloc[3 * 4 + 2] = param[2];
#endif
								g3d_matrix_multiply(matrix, mloc, matrix);
								valid_matrix = TRUE;
								break;

							case 0x01: /* rotation */
								g3d_matrix_rotate(
									(gfloat)param[3] * M_PI / 180.0,
									param[0], param[1], param[2], matrix);
								g3d_matrix_multiply(mloc, matrix, matrix);
								valid_matrix = TRUE;
								break;

							default:
								break;
						}
					}
#if DEBUG > 2
					g_print("LeoCAD: key 0x%02x (%d): "
						"%+2.2f %+2.2f %+2.2f %+2.2f\n",
						ktype, ktime,
						param[0], param[1], param[2], param[3]);
#endif
				} /* keys */

				nkeys = g3d_read_int32_le(f);
				for(i = 0; i < nkeys; i ++)
				{
					/* param */
					param[0] = g3d_read_float_le(f);
					param[1] = g3d_read_float_le(f);
					param[2] = g3d_read_float_le(f);
					param[3] = g3d_read_float_le(f);

					/* time */
					ktime = g3d_read_int16_le(f);

					/* type */
					ktype = g3d_read_int8(f);
				}
			} /* pver > 5 */
			else /* pver <= 5 */
			{
				if(pver > 2)
				{
					nkeys = g3d_read_int8(f);
					for(i = 0; i < nkeys; i ++)
					{
						if(pver > 3)
						{
							/* matrix */
							for(j = 0; j < 16; j ++)
							{
#if DEBUG > 2
								if((j % 4) == 0)
									g_print("LeoCAD: matrix:");
#endif
#if 0
								matrix[(j % 4) * 4 + j / 4] =
									g3d_read_float_le(f);
#else
								matrix[j] = g3d_read_float_le(f);
#endif
#if DEBUG > 2
								g_print(" %+.2f", matrix[j]);
								if((j % 4) == 3)
									g_print("\n");
#endif
							}

							valid_matrix = TRUE;
						}
						else
						{
							/* move: 3 x float */
							offx = g3d_read_float_le(f);
							offy = g3d_read_float_le(f);
							offz = g3d_read_float_le(f);

							/* rotate: 3 x float */
							rotx = g3d_read_float_le(f);
							roty = g3d_read_float_le(f);
							rotz = g3d_read_float_le(f);
						}

						/* time */
						g3d_read_int8(f);

						/* bl? */
						g3d_read_int32_le(f);
					} /* .. nkeys */
				} /* pver > 2 */
				else /* pver <= 2 */
				{
					/* move: 3 x float */
					offx = g3d_read_float_le(f);
					offy = g3d_read_float_le(f);
					offz = g3d_read_float_le(f);

					/* rotate: 3 x float */
					rotx = g3d_read_float_le(f);
					roty = g3d_read_float_le(f);
					rotz = g3d_read_float_le(f);
				}
			} /* pver <= 5 */
		} /* pver < 9 */

		/* common stuff */

		/* name of piece */
		fread(name, 1, 9, f);

		/* color */
		color = g3d_read_int8(f);

		if(pver < 5)
			color = leocad_library_convert_color(color);

#if DEBUG > 0
		g_print("LeoCAD: [%d]: '%-8s', color 0x%02x\n", pver, name, color);
#endif

		/* step show */
		g3d_read_int8(f);

		/* step hide */
		if(pver > 1)
			g3d_read_int8(f);

		if(pver > 5)
		{
			/* frame show */
			g3d_read_int16_le(f);
			/* frame hide */
			g3d_read_int16_le(f);

			if(pver > 7)
			{
				/* state */
				g3d_read_int8(f);

				len8 = g3d_read_int8(f);
				fseek(f, len8, SEEK_CUR);
			}
			else /* pver <= 7 */
			{
				/* hide */
				g3d_read_int32_le(f);

				fseek(f, 81, SEEK_CUR);
			} /* pver <= 7 */

			if(pver > 6)
			{
				/* group pointer ?! */
				g3d_read_int32_le(f);
			}
		} /* pver > 5 */
		else /* pver <= 5 */
		{
			/* group pointer ?! */
			g3d_read_int8(f);

			/* hide */
			g3d_read_int8(f);
		}

	} /* lcdversion > 0.4 */

	object = leocad_library_get_piece(library, name);
	if(object == NULL)
	{
		g_warning("LeoCAD: failed to load piece '%s'", name);
		return FALSE;
	}

	/* matrix */
	if(!valid_matrix)
	{
		/* translation */
#if 0
		mloc[0 * 4 + 3] = offx;
		mloc[1 * 4 + 3] = offy;
		mloc[2 * 4 + 3] = offz;
#else
		mloc[3 * 4 + 0] = offx;
		mloc[3 * 4 + 1] = offy;
		mloc[3 * 4 + 2] = offz;
#endif
		/* rotation */
		rotx = (gfloat)(rotx * M_PI) / 180.0;
		roty = (gfloat)(roty * M_PI) / 180.0;
		rotz = (gfloat)(rotz * M_PI) / 180.0;

		g3d_matrix_rotate_xyz(rotx, roty, rotz, matrix);

		/* combine */
		g3d_matrix_multiply(mloc, matrix, matrix);
	}

	/*g3d_matrix_dump(matrix);*/

	/* transform vertices */
	for(i = 0; i < object->vertex_count; i ++)
		g3d_vector_transform(
			&(object->vertex_data[i * 3 + 0]),
			&(object->vertex_data[i * 3 + 1]),
			&(object->vertex_data[i * 3 + 2]),
			matrix);

	/* change color */
	fitem = object->faces;
	while(fitem)
	{
		face = (G3DFace *)fitem->data;
		if(face->material == mat_change)
		{
			face->material = leocad_library_get_nth_material(library, color);
		}

		if(face->material == NULL)
		{
			face->material = leocad_library_get_nth_material(library, 0);
		}

		fitem = fitem->next;
	}

	/* add to model object list */
	model->objects = g_slist_append(model->objects, object);

	return TRUE;
}

static gboolean leocad_load_lcd(const gchar *filename, G3DModel *model,
	LeoCadLibrary *library, G3DContext *context)
{
	gchar magic[32];
	gfloat version;
	guint32 i, count;
	FILE *f;

	f = fopen(filename, "rb");
	if(f == NULL)
	{
		g_warning("LeoCAD: failed to open '%s'", filename);
		return EXIT_FAILURE;
	}

	fread(magic, 1, 32, f);
	if(strncmp(magic, "LeoCAD", 6) != 0)
	{
		g_warning("LeoCAD: '%s' is not a valid LeoCAD project file", filename);
		fclose(f);
		return FALSE;
	}

	sscanf(&magic[7], "%f", &version);

	if(version > 0.4)
	{
#if DEBUG > 0
		g_print("LeoCAD: file version %.1f, getting next float\n", version);
#endif
		version = g3d_read_float_le(f);
	}

#if DEBUG > 0
	g_print("LeoCAD: file version %.1f\n", version);
#endif

	/* background color */
	g3d_context_set_bgcolor(context,
		g3d_read_int8(f) / 255.0,
		g3d_read_int8(f) / 255.0,
		g3d_read_int8(f) / 255.0,
		1.0);
	g3d_read_int8(f);

	/* view */
	if(version < 0.6)
	{
		/* eye: 3 x double */
		fseek(f, 24, SEEK_CUR);

		/* target: 3 x double */
		fseek(f, 24, SEEK_CUR);
	}

	/* angle snap */
	g3d_read_int32_le(f);
	/* snap */
	g3d_read_int32_le(f);
	/* line width */
	g3d_read_float_le(f);
	/* detail */
	g3d_read_int32_le(f);
	/* cur group */
	g3d_read_int32_le(f);
	/* cur color */
	g3d_read_int32_le(f);
	/* action */
	g3d_read_int32_le(f);
	/* cur step */
	g3d_read_int32_le(f);

	if(version > 0.8)
	{
		/* scene */
		g3d_read_int32_le(f);
	}

	/* piece count */
	count = g3d_read_int32_le(f);
	for(i = 0; i < count; i ++)
	{
		/* load piece */
		leocad_load_lcd_piece(f, model, library, version);
	}

	fclose(f);

	return TRUE;
}
