/* $Id: imp_leocad_library.c,v 1.1.2.3 2006/01/23 17:03:06 dahms Exp $ */

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

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include <glib.h>

#include <g3d/types.h>
#include <g3d/object.h>
#include <g3d/material.h>
#include <g3d/read.h>
#include <g3d/primitive.h>
#include <g3d/matrix.h>

#include "imp_leocad_library.h"

#define LEOCAD_FLAG_PIECE_COUNT         0x01
#define LEOCAD_FLAG_PIECE_LONGDATA      0x02
#define LEOCAD_FLAG_PIECE_CCW           0x04
#define LEOCAD_FLAG_PIECE_SMALL         0x10
#define LEOCAD_FLAG_PIECE_MEDIUM        0x20
#define LEOCAD_FLAG_LONGDATA_RUNTIME    0x40

#define LEOCAD_TYPE_MESH                0x01
#define LEOCAD_TYPE_STUD                0x02
#define LEOCAD_TYPE_STUD2               0x03
#define LEOCAD_TYPE_STUD3               0x04
#define LEOCAD_TYPE_STUD4               0x05

static gboolean leocad_library_read_pieces_idx(LeoCadLibrary *library,
	FILE *idx);
static gboolean leocad_create_materials(LeoCadLibrary *library);

struct LeoCadConnection {
	guint8 type;
	gfloat center[3];
	gfloat normal[3];
};

LeoCadLibrary *leocad_library_load(const gchar *libdir)
{
	LeoCadLibrary *library;
	gchar filename[1025];
	FILE *idx, *bin;

	library = g_new0(LeoCadLibrary, 1);

	sprintf(filename, "%s/%s", libdir, "pieces.idx");
	idx = fopen(filename, "rb");
	if(idx == NULL)
	{
		g_print("LeoCAD: failed to read '%s'\n", filename);
		g_free(library);
		return NULL;
	}

	sprintf(filename, "%s/%s", libdir, "pieces.bin");
	bin = fopen(filename, "rb");
	if(bin == NULL)
	{
		g_print("LeoCAD: failed to read '%s'\n", filename);
		fclose(idx);
		g_free(library);
		return NULL;
	}

	library->pieces = g_hash_table_new(g_str_hash, g_str_equal);
	library->pieces_bin = bin;

	leocad_library_read_pieces_idx(library, idx);
	fclose(idx);

	leocad_create_materials(library);

	return library;
}

void leocad_library_free(LeoCadLibrary *library)
{
	/* remove materials */
	/* remove pieces */
	/* FIXME: remove pieces */
	g_hash_table_destroy(library->pieces);

	/* free library */
	g_free(library);
}

static gfloat leocad_read_scaled16(FILE *f, gfloat scale)
{
	gint16 x;

	x = g3d_read_int16_le(f);

	return (gfloat)(x / scale);
}

static gboolean leocad_create_materials(LeoCadLibrary *library)
{
	const guint8 colors[] = {
		 166,  25,  25, 255,  /*  0 - Red */
		 255, 127,  51, 255,  /*  1 - Orange */
		  25, 102,  25, 255,  /*  2 - Green */
		  76, 153,  76, 255,  /*  3 - Light Green */
		   0,  51, 178, 255,  /*  4 - Blue */
		  51, 102, 229, 255,  /*  5 - Light Blue */
		 204, 204,   0, 255,  /*  6 - Yellow */
		 242, 242, 242, 255,  /*  7 - White */
		  76,  76,  76, 255,  /*  8 - Dark Gray */
		  25,  25,  25, 255,  /*  9 - Black */
		 102,  51,  51, 255,  /* 10 - Brown */
		 178,  76, 153, 255,  /* 11 - Pink */
		 153,  51, 153, 255,  /* 12 - Purple */
		 229, 178,  51, 255,  /* 13 - Gold */
		 153,  25,  25, 153,  /* 14 - Clear Red */
		 255, 153,  76, 153,  /* 15 - Clear Orange */
		  25, 102,  25, 153,  /* 16 - Clear Green */
		 153, 178,  76, 153,  /* 17 - Clear Light Green */
		   0,   0, 127, 153,  /* 18 - Clear Blue */
		  51, 102, 229, 153,  /* 19 - Clear Light Blue */
		 229, 229,   0, 153,  /* 20 - Clear Yellow */
		 229, 229, 229, 153,  /* 21 - Clear White */
		 127, 127, 127, 255,  /* 22 - Light Gray */
		 204, 204, 178, 255,  /* 23 - Tan */
		 153, 102, 102, 255,  /* 24 - Light Brown */
		 229, 178, 229, 255,  /* 25 - Light Pink */
		  25, 178, 204, 255,  /* 26 - Turquoise */
		 204, 204, 204, 255,  /* 27 - Silver */
		  51,  51,  51, 255,  /* 28 - Edges */
		 229,  76, 102, 255,  /* 29 - Selected */
		 102,  76, 229, 255}; /* 30 - Focused */

	G3DMaterial *material;
	guint32 i;

	for(i = 0; i < 31; i ++)
	{
		material = g3d_material_new();
		material->r = (gfloat)colors[i * 4 + 0] / 255.0;
		material->g = (gfloat)colors[i * 4 + 1] / 255.0;
		material->b = (gfloat)colors[i * 4 + 2] / 255.0;
		material->a = (gfloat)colors[i * 4 + 3] / 255.0;

		library->materials = g_slist_append(library->materials, material);
	}

	return TRUE;
}

guint8 leocad_library_convert_color(guint8 n)
{
	static guint8 converted_colors[20] =
		{ 0,2,4,9,7,6,22,8,10,11,14,16,18,9,21,20,22,8,10,11 };

	if(n >= 20)
		return 0;

	return converted_colors[n];
}

G3DMaterial *leocad_library_get_nth_material(LeoCadLibrary *library, guint8 n)
{
	return g_slist_nth_data(library->materials, n);
}

G3DObject *leocad_library_get_piece(LeoCadLibrary *library, const gchar *name)
{
	LeoCadPiece *piece;
	G3DObject *stud;
	G3DFace *face;
	guint32 i, j, k, nconn, ngrp, ntex, ngrpconn, ncol, nx, color = 0;
	guint16 grp_type;
	gfloat scale = 100.0;
	gfloat matrix[16];
	FILE *bin;
	struct LeoCadConnection *connections;

	piece = g_hash_table_lookup(library->pieces, name);

	if(piece == NULL)
	{
		g_warning("LeoCAD: failed to load piece '%s'", name);
		return NULL;
	}

	bin = library->pieces_bin;

	if(piece->object == NULL)
	{
		piece->object = g_new0(G3DObject, 1);
		piece->object->name = g_strdup(piece->description);

		fseek(bin, piece->offset_bin, SEEK_SET);
		piece->object->vertex_count = g3d_read_int32_le(bin);
		piece->object->vertex_data = g_new0(gfloat,
			piece->object->vertex_count * 3);

		if(piece->flags & LEOCAD_FLAG_PIECE_SMALL)
			scale = 10000.0;
		else if(piece->flags & LEOCAD_FLAG_PIECE_MEDIUM)
			scale = 1000.0;

		for(i = 0; i < piece->object->vertex_count; i ++)
		{
			piece->object->vertex_data[i * 3 + 0] =
				leocad_read_scaled16(bin, scale);
			piece->object->vertex_data[i * 3 + 1] =
				leocad_read_scaled16(bin, scale);
			piece->object->vertex_data[i * 3 + 2] =
				leocad_read_scaled16(bin, scale);
		}
#if DEBUG > 1
		g_print("LeoCAD: piece '%s': flags 0x%02X\n",
			name, piece->flags);
#endif

#if DEBUG > 1
		g_print("LeoCAD: piece '%s': %d vertices\n",
			name, piece->object->vertex_count);
#endif

		/* connections */
		nconn = g3d_read_int16_le(bin);
#if DEBUG > 1
		g_print("LeoCAD: piece '%s': %d connections\n", name, nconn);
#endif
		connections = g_new0(struct LeoCadConnection, nconn);
		for(i = 0; i < nconn; i ++)
		{
			connections[i].type = g3d_read_int8(bin);

			/* center */
			connections[i].center[0] = leocad_read_scaled16(bin, scale);
			connections[i].center[1] = leocad_read_scaled16(bin, scale);
			connections[i].center[2] = leocad_read_scaled16(bin, scale);

			/* normal */
			connections[i].normal[0] = g3d_read_int16_le(bin) / (1 << 14);
			connections[i].normal[1] = g3d_read_int16_le(bin) / (1 << 14);
			connections[i].normal[2] = g3d_read_int16_le(bin) / (1 << 14);
		}

		/* textures */
		ntex = g3d_read_int8(bin);
#if DEBUG > 0
		if(ntex > 0)
			g_print("LeoCAD: piece '%s': %d textures\n", name, ntex);
#endif
		for(i = 0; i < ntex; i ++)
		{
			/* TODO: */
		}

		/* groups */
		ngrp = g3d_read_int16_le(bin);
#if DEBUG > 1
		g_print("LeoCAD: piece '%s': %d groups @ 0x%08lx\n",
			name, ngrp, ftell(bin));
#endif
		for(i = 0; i < ngrp; i ++)
		{
			/* group connections */
			ngrpconn = g3d_read_int8(bin);
			for(j = 0; j < ngrpconn; j ++)
			{
				g3d_read_int16_le(bin);
			}

#if 0
			while(1)
			{
#endif
				grp_type = g3d_read_int8(bin);
				if(grp_type == 0)
				{
					break;
				}
#if DEBUG > 1
				g_print("LeoCAD: piece '%s': grp %d: type 0x%02x @ 0x%08lx\n",
					name, i, grp_type, ftell(bin));
#endif

				switch(grp_type)
				{
					case LEOCAD_TYPE_MESH:
						ncol = g3d_read_int16_le(bin);
#if DEBUG > 1
						g_print("LeoCAD: piece '%s': grp %d: %d colors "
							"(@ 0x%08lx)\n",
							name, i, ncol, ftell(bin));
#endif
						for(j = 0; j < ncol; j ++)
						{
							/* color code */
							color = g3d_read_int16_le(bin);
#if DEBUG > 1
							g_print(
								"LeoCAD: piece '%s': grp %d: color 0x%04x\n",
								name, i, color);
#endif
							/* quads? */
							nx = g3d_read_int16_le(bin);
							for(k = 0; k < nx / 4; k ++)
							{
								face = g_new0(G3DFace, 1);
								face->material =
									g_slist_nth_data(
										library->materials, color);
								face->vertex_count = 4;
								face->vertex_indices = g_new0(guint32, 4);

								face->vertex_indices[0] =
									g3d_read_int16_le(bin);
								face->vertex_indices[1] =
									g3d_read_int16_le(bin);
								face->vertex_indices[2] =
									g3d_read_int16_le(bin);
								face->vertex_indices[3] =
									g3d_read_int16_le(bin);

								piece->object->faces = g_slist_prepend(
									piece->object->faces, face);
							}
#if DEBUG > 1
							g_print("LeoCAD: piece '%s': grp %d: "
								"quads: %d bytes\n",
								name, i, nx * 2);
#endif
							nx = g3d_read_int16_le(bin);
							for(k = 0; k < nx / 3; k ++)
							{
								face = g_new0(G3DFace, 1);
								face->material =
									g_slist_nth_data(
										library->materials, color);
								face->vertex_count = 3;
								face->vertex_indices = g_new0(guint32, 3);

								face->vertex_indices[0] =
									g3d_read_int16_le(bin);
								face->vertex_indices[1] =
									g3d_read_int16_le(bin);
								face->vertex_indices[2] =
									g3d_read_int16_le(bin);

								piece->object->faces = g_slist_prepend(
									piece->object->faces, face);
							}

#if DEBUG > 1
							g_print("LeoCAD: piece '%s': grp %d: "
								"triangles: %d bytes\n",
								name, i, nx * 2);
#endif
							nx = g3d_read_int16_le(bin);
#if DEBUG > 1
							g_print("LeoCAD: piece '%s': grp %d: "
								"skipping %d bytes @ 0x%08lx\n",
								name, i, nx * 2, ftell(bin));
#endif
							fseek(bin, nx * 2, SEEK_CUR);
						}
						break;

					case LEOCAD_TYPE_STUD:
					case LEOCAD_TYPE_STUD2:
					case LEOCAD_TYPE_STUD3:
					case LEOCAD_TYPE_STUD4:
						stud = NULL;
						color = g3d_read_int8(bin);
#if DEBUG > 0
						g_print("LeoCAD: piece '%s': stud 0x%02x\n",
							name, grp_type);
#endif
						if(grp_type == LEOCAD_TYPE_STUD4)
							stud = g3d_primitive_tube(
								0.24, /* inner radius */
								0.32, /* outer radius */
								0.16, /* height */
								16, /* sides */
								TRUE, FALSE, /* top, bottom */
								leocad_library_get_nth_material(
									library, color));

						g3d_matrix_identity(matrix);
						for(j = 0; j < 12; j ++)
							matrix[(j % 3) * 4 + (j / 3)] =
								g3d_read_float_le(bin);

						if(stud && piece->object)
						{
							g3d_object_transform(stud, matrix);
							g3d_object_merge(piece->object, stud);
							stud = NULL;
						}

						break;

					default:
#if DEBUG > 1
						g_print(
							"LeoCAD: piece '%s': unhandled group type 0x%02x "
							"@ 0x%08lx\n",
							name, grp_type, ftell(bin));
#endif
						break;
				} /* grp_type */
#if 0
			} /* grp_type != 0 */
#endif
			g3d_read_int8(bin);
		} /* ngrp */

		/* generate studs */
		for(i = 0; i < nconn; i ++)
		{
#if DEBUG > 4
			g_print("LeoCAD: connection %d: type 0x%02x\n",
				i, connections[i].type);
#endif

			switch(connections[i].type)
			{
				case 0:
					stud = g3d_primitive_cylinder(
						0.24, /* radius */
						0.16, /* height */
						16, /* sides */
						TRUE, FALSE, /* top, bottom */
						leocad_library_get_nth_material(library, color));
					break;

				case 2:
					stud = g3d_primitive_tube(
						0.24, /* inner radius */
						0.32, /* outer radius */
						0.16, /* height */
						16, /* sides */
						FALSE, TRUE, /* top, bottom */
						leocad_library_get_nth_material(library, color));
					break;

				default:
					stud = NULL;
					break;
			}

			if(stud)
			{
#if DEBUG > 5
				g_print("LeoCAD: stud\n");
#endif
				/* transform stud */
				for(j = 0; j < stud->vertex_count; j ++)
				{
#if 0
					g3d_vector_transform(
						&(stud->vertices[j * 3 + 0]),
						&(stud->vertices[j * 3 + 1]),
						&(stud->vertices[j * 3 + 2]),
						matrix);
#endif

					stud->vertex_data[j * 3 + 0] += connections[i].center[0];
					stud->vertex_data[j * 3 + 1] += connections[i].center[1];
					stud->vertex_data[j * 3 + 2] += connections[i].center[2];
				}

				/* merge stud */
				g3d_object_merge(piece->object, stud);
			} /* stud */
		} /* nconn */

		if(connections)
			g_free(connections);
	}

	return g3d_object_duplicate(piece->object);
}

static gboolean leocad_library_read_piece(LeoCadLibrary *library, FILE *idx)
{
	gchar buffer[128];
	guint32 i;
	LeoCadPiece *piece;

	piece = g_new0(LeoCadPiece, 1);

	fread(buffer, 1, 8, idx);
	buffer[8] = '\0';
	piece->name = g_strdup(buffer);

	fread(buffer, 1, 64, idx);
	buffer[64] = '\0';
	piece->description = g_strdup(buffer);

	for(i = 0; i < 6; i ++)
		piece->bounding_box[i] = g3d_read_int16_le(idx);

	piece->flags = g3d_read_int8(idx);
	piece->default_group = g3d_read_int32_le(idx);
	piece->offset_bin = g3d_read_int32_le(idx);
	piece->info_size = g3d_read_int32_le(idx);

#if DEBUG > 1
	g_print("LeoCAD: %-8s: @ 0x%08x, %s\n",
		piece->name, piece->offset_bin, piece->description);
#endif

	g_hash_table_insert(library->pieces, piece->name, piece);

	return TRUE;
}

static gboolean leocad_library_read_pieces_idx(LeoCadLibrary *library,
	FILE *idx)
{
	gchar magic[32], nameold[9], namenew[9];
	guint8 version, lastupdate;
	guint32 nmoved, nbinsize, npieces, i;
	LeoCadPiece *piece;

	fread(magic, 1, 32, idx);
	if(strncmp(magic, "LeoCAD piece library index file", 31) != 0)
	{
		g_print("LeoCAD: pieces.idx: wrong magic\n");
		return FALSE;
	}

	version = g3d_read_int8(idx);
	lastupdate = g3d_read_int8(idx);

	fseek(idx, -8, SEEK_END);
	nmoved = g3d_read_int16_le(idx);
	nbinsize = g3d_read_int32_le(idx);
	npieces = g3d_read_int16_le(idx);

	fseek(idx, 34, SEEK_SET);

#if DEBUG > 0
	g_print("LeoCAD: pieces.idx: version %d, last update %d\n",
		version, lastupdate);
	g_print("LeoCAD: pieces.idx: %d pieces, %d moves, pieces.bin %d bytes\n",
		npieces, nmoved, nbinsize);
#endif

	for(i = 0; i < npieces; i ++)
	{
		leocad_library_read_piece(library, idx);
	}

	for(i = 0; i < nmoved; i ++)
	{
		memset(nameold, 0, 9);
		memset(namenew, 0, 9);

		fread(nameold, 1, 8, idx);
		fread(namenew, 1, 8, idx);

		piece = g_hash_table_lookup(library->pieces, namenew);
		if(piece)
			g_hash_table_insert(library->pieces, g_strdup(nameold), piece);
	}

	return TRUE;
}

