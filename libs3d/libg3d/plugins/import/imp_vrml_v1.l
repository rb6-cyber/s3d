/* $Id: imp_vrml_v1.l,v 1.1.2.2 2006/01/23 17:03:06 dahms Exp $ */

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

%option reentrant nodefault
%option header-file="imp_vrml_v1.h"
%option prefix="vrml_v1_yy"
%S VRMLHEADER
%S COMMENT

	#include <stdio.h>
	#include <string.h>
	#include <locale.h>

	#include <g3d/types.h>
	#include <g3d/material.h>
	#include <g3d/vector.h>

	#if DEBUG > 0
	#	define vrml_dump_hier(level, section) g_print("VRML: %.*s[%s]\n", \
			level * 3, "               ", section)
	#else
	#	define vrml_dump_hier(level, section) /* */
	#endif

	#define VRML_OTYPE_MAT                             0x0100
	#define VRML_OTYPE_MAT_AMBIENTCOLOR                0x0101
	#define VRML_OTYPE_MAT_DIFFUSECOLOR                0x0102
	#define VRML_OTYPE_MAT_SPECULARCOLOR               0x0103
	#define VRML_OTYPE_MAT_EMISSIVECOLOR               0x0104
	#define VRML_OTYPE_MAT_SHININESS                   0x0105
	#define VRML_OTYPE_MAT_TRANSPARENCY                0x0106

	#define VRML_OTYPE_COORD3                          0x0200
	#define VRML_OTYPE_COORD3_POINT                    0x0201

	#define VRML_OTYPE_IDXFACESET                      0x0300
	#define VRML_OTYPE_IDXFACESET_COORDIDX             0x0301
	#define VRML_OTYPE_IDXFACESET_MATIDX               0x0302

	#define VRML_OTYPE_MTRANS                          0x0400
	#define VRML_OTYPE_MTRANS_MATRIX                   0x0401

%%
	G3DObject *object = NULL;
	G3DFace *face;
	G3DMaterial *material = NULL;
	guint32 level = 0, off, otype = 0, sep_level = 0;
	guint32 ohasmtrx = 0;
	guint32 faceidx = 0, faceidxbuf[128];
	guint32 matidx = 0;
	gint32 tmps32, i;
	gfloat matrix[16];

<INITIAL>^\s*\# {
	#if DEBUG > 0
	g_print("VRML: comment:");
	#endif
	BEGIN COMMENT;
}

<INITIAL>^\#VRML/\ V {
	/* magic header */
	#if DEBUG > 0
	g_print("VRML: got header\n");
	#endif
	BEGIN VRMLHEADER;
}

<VRMLHEADER>V[0-9.]+ {
	#if DEBUG > 0
	g_print("VRML: version %s\n", yytext + 1);
	#endif
}

<VRMLHEADER>. /* */

<VRMLHEADER>\n {
	BEGIN INITIAL;
}

<COMMENT>. {
	#if DEBUG > 0
	g_print("%s", yytext);
	#endif
}

<COMMENT>\n {
	#if DEBUG > 0
	g_print("\n");
	#endif
	BEGIN INITIAL;
}

Separator/\ *\{ {
	vrml_dump_hier(level, "Separator");
	if((object == NULL) || (object->vertex_count == 0))
	{
		object = g_new0(G3DObject, 1);
		((G3DModel *)yyextra)->objects = g_slist_append(
			((G3DModel *)yyextra)->objects, object);
		sep_level = level;
		ohasmtrx = 0;
	}
}

Group/[[:space:]]*\{ {
	vrml_dump_hier(level, "Group");
}

Material/\ *\{ {
	vrml_dump_hier(level, "Material");
	if(object)
	{
		material = g3d_material_new();
		material->name = g_strdup("object material");
		object->materials = g_slist_append(object->materials, material);
	}
	otype = VRML_OTYPE_MAT;
}

MatrixTransform/[[:space:]]*\{ {
	vrml_dump_hier(level, "MatrixTransform");
	otype = VRML_OTYPE_MTRANS;
}

Coordinate3/\ *\{ {
	vrml_dump_hier(level, "Coordinate3");
	if(!object)
	{
		object = g_new0(G3DObject, 1);
		object->name = g_strdup("VRML 1 object");
		((G3DModel *)yyextra)->objects = g_slist_append(
			((G3DModel *)yyextra)->objects, object);
		ohasmtrx = 0;
	}
	otype = VRML_OTYPE_COORD3;
}

IndexedFaceSet/\ *\{ {
	vrml_dump_hier(level, "IndexedFaceSet");
	otype = VRML_OTYPE_IDXFACESET;
}

-?[0-9]+[0-9e \.-]*/[,\]] {
	#if DEBUG > 3
	g_print("# (0x%04x) %s\n", otype, yytext);
	#endif
	if((otype & 0xFF00) == VRML_OTYPE_MAT)
	{
		if(object == NULL) continue;
		material = g_slist_nth_data(object->materials, matidx);
		if(material == NULL)
		{
			material = g3d_material_new();
			material->name = g_strdup_printf("material #%d", matidx);
			object->materials = g_slist_append(object->materials, material);
		}
		matidx ++;
		switch(otype)
		{
			case VRML_OTYPE_MAT_DIFFUSECOLOR:
				if(sscanf(yytext, "%f%f%f",
					&(material->r), &(material->g), &(material->b)) != 3)
				{
		#if DEBUG > 0
					g_print("VRML1: failed to get ambient color");
		#endif
				}
				break;

			default:
	#if DEBUG > 0
				g_print("VRML: unhandled material property: 0x%02x (%s)\n",
					otype, yytext);
	#endif
				break;
		}
	}
	else if(otype == VRML_OTYPE_COORD3_POINT)
	{
		off = object->vertex_count;
		object->vertex_count ++;
		object->vertex_data = g_realloc(object->vertex_data,
			object->vertex_count * 3 * sizeof(gfloat));

	#if DEBUG > 3
		g_print("VRML: object vertex count: %d\n", object->vertex_count);
	#endif

		if(sscanf(yytext, "%f%f%f",
			&(object->vertex_data[off * 3 + 0]),
			&(object->vertex_data[off * 3 + 1]),
			&(object->vertex_data[off * 3 + 2])) != 3)
		{
			g_warning("VRML: failed to read vertex (%s)\n", yytext);
		}
		else
		{
	#if DEBUG > 3
			if((object->vertex_data[off * 3 + 0] == 0.0) ||
				(object->vertex_data[off * 3 + 1] == 0.0) ||
				(object->vertex_data[off * 3 + 2] == 0.0))
			{
				g_print("VRML1: 0.0: %+2.2f %+2.2f %+2.2f (%s)\n",
					object->vertex_data[off * 3 + 0],
					object->vertex_data[off * 3 + 1],
					object->vertex_data[off * 3 + 2],
					yytext);
			}
	#endif
			if(ohasmtrx)
				g3d_vector_transform(
					&(object->vertex_data[off * 3 + 0]),
					&(object->vertex_data[off * 3 + 1]),
					&(object->vertex_data[off * 3 + 2]),
					matrix);
		}
	}
	else if(otype == VRML_OTYPE_IDXFACESET_COORDIDX)
	{
		sscanf(yytext, "%i", &tmps32);
		if(tmps32 == -1)
		{
			if(object == NULL) continue;

			face = g_new0(G3DFace, 1);
			face->material = material;
			if(face->material == NULL)
				face->material = g_slist_nth_data(
					object->materials, 0);
			if(face->material == NULL)
				face->material = g_slist_nth_data(
					((G3DModel *)yyextra)->materials, 0);
			face->vertex_count = faceidx;
			face->vertex_indices = g_new0(guint32, face->vertex_count);
			for(i = 0; i < face->vertex_count; i ++)
			{
				if(faceidxbuf[i] >= object->vertex_count)
				{
					g_print("VRML: Face: index %d >= vertex count (%d)\n",
						faceidxbuf[i], object->vertex_count);
				}
				else
				{
					face->vertex_indices[i] = faceidxbuf[i];
				}
			}

			if(face->vertex_count >= 3)
				object->faces = g_slist_prepend(object->faces, face);
			faceidx = 0;
		}
		else
		{
	#if DEBUG > 3
			if(tmps32 == 0)
				g_print("VRML1: faceidx 0: %s\n", yytext);
	#endif
			faceidxbuf[faceidx] = tmps32;
			faceidx ++;
		}
	}
	else if(otype == VRML_OTYPE_IDXFACESET_MATIDX)
	{
		i = atoi(yytext);
	#if DEBUG > 3
		g_print("VRML1: looking for object material #%d\n", i);
	#endif
		material = g_slist_nth_data(object->materials, i);
		if(material)
		{
	#if DEBUG > 3
			g_print("VRML1: got material to update\n");
	#endif
			face = g_slist_nth_data(object->faces, faceidx);
			if(face)
			{
				face->material = material;
	#if DEBUG > 1
				g_print("VRML1: updating material of face #%d\n", faceidx);
	#endif
			}
		}
		faceidx ++;
	}
}

-?[0-9]+[[:space:]0-9\.-]* {
	if(otype == VRML_OTYPE_MTRANS_MATRIX)
	{
		if(sscanf(yytext, "%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f",
			&matrix[0 * 4 + 0],
			&matrix[0 * 4 + 1],
			&matrix[0 * 4 + 2],
			&matrix[0 * 4 + 3],

			&matrix[1 * 4 + 0],
            &matrix[1 * 4 + 1],
            &matrix[1 * 4 + 2],
            &matrix[1 * 4 + 3],

			&matrix[2 * 4 + 0],
            &matrix[2 * 4 + 1],
            &matrix[2 * 4 + 2],
            &matrix[2 * 4 + 3],

			&matrix[3 * 4 + 0],
            &matrix[3 * 4 + 1],
            &matrix[3 * 4 + 2],
            &matrix[3 * 4 + 3]) != 16)
		{
			g_warning("VRML: failed to read matrix line (%s)", yytext);
		}
	}
}

ambientColor/[[:space:]]*\[ {
	otype = VRML_OTYPE_MAT_AMBIENTCOLOR;
}

diffuseColor/[[:space:]]*\[ {
	otype = VRML_OTYPE_MAT_DIFFUSECOLOR;
}

specularColor/[[:space:]]*\[ {
	otype = VRML_OTYPE_MAT_SPECULARCOLOR;
}

emissiveColor/[[:space:]]*\[ {
	otype = VRML_OTYPE_MAT_EMISSIVECOLOR;
}

shininess/[[:space:]]*\[ {
	otype = VRML_OTYPE_MAT_SHININESS;
}

transparency/[[:space:]]*\[ {
	otype = VRML_OTYPE_MAT_TRANSPARENCY;
}

point/[[:space:]]*\[ {
	otype = VRML_OTYPE_COORD3_POINT;
}

coordIndex/[[:space:]]*\[ {
	if(otype == VRML_OTYPE_IDXFACESET)
		otype = VRML_OTYPE_IDXFACESET_COORDIDX;
}

materialIndex/[[:space:]]*\[ {
	#if DEBUG > 0
	g_print("VRML1: materialIndex\n");
	#endif
	/*if(otype == VRML_OTYPE_IDXFACESET)*/
		otype = VRML_OTYPE_IDXFACESET_MATIDX;
}

matrix {
	if(otype == VRML_OTYPE_MTRANS)
	{
		otype = VRML_OTYPE_MTRANS_MATRIX;
		ohasmtrx = 1;
	}
}

\] {
	/* set to parent object */
	otype &= 0xFF00;
	matidx = 0;
	faceidx = 0;
}

[A-Z][[:alnum:]]*/\ *\{ {
	vrml_dump_hier(level, yytext);
}

\{ {
	level ++;
}

\} {
	#if DEBUG > 3
	g_print("}\n");
	#endif
	level --;

	if(material)
		material = NULL;

	if(object && (sep_level == level))
	{
		object = NULL;
	}

	/* reset object type */
	otype = 0x0000;
}

. /* */
\n /* */

<<EOF>> {
	yyterminate();
}

%%

