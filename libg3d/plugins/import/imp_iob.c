/* $Id: imp_iob.c,v 1.1.2.2 2006/01/23 17:03:06 dahms Exp $ */

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

#include <glib.h>

#include <g3d/types.h>
#include <g3d/context.h>
#include <g3d/material.h>
#include <g3d/read.h>
#include <g3d/iff.h>

int iob_read_directory(FILE *f, guint32 nbytes, G3DModel *model,
	void *pobject, guint32 parentid, int level, G3DContext *context);

gboolean plugin_load_model(G3DContext *context, const gchar *filename,
	G3DModel *model, gpointer user_data)
{
	guint32 id, len;
	FILE *f;

	f = g3d_iff_open(filename, &id, &len);
	if(id != G3D_IFF_MKID('T','D','D','D'))
	{
		g_warning("file is not an .iob (TDDD) file %s", filename);
		fclose(f);
		return FALSE;
	}

	iob_read_directory(f, len, model, NULL, id, 1, context);

	return TRUE;
}

char *plugin_description(void)
{
	return g_strdup(
		"import plugin for Impulse Turbo Silver / Imagine objects\n");
}

char **plugin_extensions(void)
{
	return g_strsplit("iob", ":", 0);
}


/*****************************************************************************/
/* IOB specific                                                              */
/*****************************************************************************/


int iob_read_pnts(FILE *f, int nbytes, G3DObject* object, int type);
int *iob_read_edges(FILE *f, int nbytes, G3DObject* object, int type);
int iob_read_mat_lists(FILE *f, int nbytes, G3DObject* object, int type);
int iob_read_faces(FILE *f, int nbytes, G3DObject* object, int *edges,
	int type);
gfloat iob_read_fract(FILE *f);

int iob_read_directory(FILE *f, guint32 nbytes, G3DModel *model,
	void *pobject, guint32 parentid, int level, G3DContext *context)
{
	guint32 id, len;
	gint32 toread = nbytes;
#if DEBUG > 0
	int i;
#endif
	int *iob_edges = NULL;
	guint32 typeShape, typeLamp;
	gdouble offx, offy, offz;
	G3DObject *iob_object = NULL;
	G3DMaterial *material;

	while(toread > 0)
	{
		toread -= g3d_iff_readchunk(f, &id, &len);
#if DEBUG > 0
		for(i=0; i<level; i++) g_printerr("  ");
		g_printerr("[%c%c%c%c] %d bytes\n",
			(id >> 24) & 0xFF, (id >> 16) & 0xFF,
			(id >> 8) & 0xFF, id & 0xFF, len);
#endif
		switch(id)
		{
			case G3D_IFF_MKID('A','X','I','S'):
				/* direction vectors for coordinate system */
				/* x */
				iob_read_fract(f);
				iob_read_fract(f);
				iob_read_fract(f);
				/* y */
				iob_read_fract(f);
				iob_read_fract(f);
				iob_read_fract(f);
				/* z */
				iob_read_fract(f);
				iob_read_fract(f);
				iob_read_fract(f);
				break;

			case G3D_IFF_MKID('O','B','J',' '):
				iob_read_directory(f, len, model, NULL, id, level+1, context);
				break;

			case G3D_IFF_MKID('D','E','S','C'):
				iob_object = g_new0(G3DObject, 1);
				model->objects = g_slist_append(model->objects, iob_object);
				material = g3d_material_new();
				material->flags |= G3D_FLAG_MAT_TWOSIDE;
				iob_object->materials = g_slist_append(iob_object->materials,
					material);
				material->name = g_strdup("material");
				iob_read_directory(f, len, model, iob_object, id, level+1,
					context);
				break;

			case G3D_IFF_MKID('N','A','M','E'):
				((G3DObject*)pobject)->name = g_malloc0(19);
				fread(((G3DObject*)pobject)->name, 1, len, f);
				break;

			case G3D_IFF_MKID('S','H','P','2'):
				typeShape = g3d_read_int16_be(f);
				typeLamp  = g3d_read_int16_be(f);
#if DEBUG > 3
				g_printerr("shapes: shape: 0x%4.4X lamp 0x%4.4X\n",
					 typeShape, typeLamp);
#endif
				break;

			case G3D_IFF_MKID('P','O','S','I'):
				offx = (gdouble)g3d_read_int32_be(f) / 0xFFFF;
				offy = (gdouble)g3d_read_int32_be(f) / 0xFFFF;
				offz = (gdouble)g3d_read_int32_be(f) / 0xFFFF;
				break;

			case G3D_IFF_MKID('P','N','T','S'):
			case G3D_IFF_MKID('P','N','T','2'):
				iob_read_pnts(f,len,(G3DObject*)pobject, id);
				break;

			case G3D_IFF_MKID('E','D','G','E'):
			case G3D_IFF_MKID('E','D','G','2'):
				iob_edges = iob_read_edges(f, len, (G3DObject*)pobject, id);
				break;

			case G3D_IFF_MKID('F','A','C','E'):
			case G3D_IFF_MKID('F','A','C','2'):
				iob_read_faces(f, len, (G3DObject*)pobject, iob_edges, id);
				break;

#if 0
			/* too slow and maybe buggy for now */
			case G3D_IFF_MKID('C','L','S','T'):
			case G3D_IFF_MKID('R','L','S','T'):
			case G3D_IFF_MKID('T','L','S','T'):
			case G3D_IFF_MKID('C','L','S','2'):
			case G3D_IFF_MKID('R','L','S','2'):
			case G3D_IFF_MKID('T','L','S','2'):
				iob_read_mat_lists(f, len, (G3DObject*)pobject, id);
				break;
#endif

			case G3D_IFF_MKID('C','O','L','R'):
				material = g_slist_nth_data(((G3DObject*)pobject)->materials,
					0);
				g3d_read_int8(f);
				material->r = (float)g3d_read_int8(f) / 255.0;
				material->g = (float)g3d_read_int8(f) / 255.0;
				material->b = (float)g3d_read_int8(f) / 255.0;
#if DEBUG > 3
				g_printerr("rgb: %f,%f,%f\n", material->r, material->g,
					material->b);
#endif
				break;
			case G3D_IFF_MKID('R','E','F','L'): {
				int r,g,b;
				material = g_slist_nth_data(((G3DObject*)pobject)->materials,
					0);
				g3d_read_int8(f);
				r = g3d_read_int8(f);
				g = g3d_read_int8(f);
				b = g3d_read_int8(f);
				material->specular[0] = (float)r / 255.0;
				material->specular[1] = (float)g / 255.0;
				material->specular[2] = (float)b / 255.0;
				} break;

			case G3D_IFF_MKID('T','O','B','J'):
				/* end of child objects */
				break;

			case G3D_IFF_MKID('T','R','A','N'): {
				int r,g,b;
				material = g_slist_nth_data(((G3DObject*)pobject)->materials,
					0);
				g3d_read_int8(f);
				r = g3d_read_int8(f);
				g = g3d_read_int8(f);
				b = g3d_read_int8(f);
#if DEBUG > 3
				g_printerr("rgb: %d,%d,%d\n", r, g, b);
#endif
				material->a = 1.0 - ((float)r / 255.0);
				} break;

			default:
#if DEBUG > 0
				g_printerr("*** unhandled ***\n");
#endif
				fseek(f, len+(len%2), SEEK_CUR);
		}

		g3d_context_update_interface(context);
	}
	if(iob_edges != NULL) g_free(iob_edges);
	return TRUE;
}

gfloat iob_read_fract(FILE *f)
{
	gint32 i = g3d_read_int32_be(f);
	return (gfloat)(i / 0xFFFF);
}

int iob_read_pnts(FILE *f, int nbytes, G3DObject* object, int type)
{
	int i, toread = nbytes;

	g_return_val_if_fail(object != NULL, FALSE);

	if(type == G3D_IFF_MKID('P','N','T','S'))
	{
		object->vertex_count = g3d_read_int16_be(f);
		toread -= 2;
	}
	else
	{
		object->vertex_count = g3d_read_int32_be(f);
		toread -= 4;
	}
	object->vertex_data = g_new0(gfloat, object->vertex_count * 3);
#if DEBUG > 3
	g_printerr("iob_read_pnts: malloc'ed object->vertices (%d vertices)\n",
						 object->vertex_count);
#endif
	for(i=0; i<object->vertex_count; i++)
	{
		object->vertex_data[i * 3 + 0] = iob_read_fract(f);
		object->vertex_data[i * 3 + 1] = iob_read_fract(f);
		object->vertex_data[i * 3 + 2] = iob_read_fract(f);
		toread -= 12;
#if DEBUG > 3
	g_printerr("iob_read_pnts: reading vertex #%d (%le|%le|%le)\n", i+1,
		object->vertex_data[i*3+0], object->vertex_data[i*3+1],
		object->vertex_data[i*3+2]);
#endif
	}
	if(toread != 0)
	{
		g_printerr("error in PNTS chunk (%d bytes to read)\n", toread);
		fseek(f, toread, SEEK_CUR);
		return FALSE;
	}
	return TRUE;
}

int *iob_read_edges(FILE *f, int nbytes, G3DObject* object, int type)
{
	int i, nedges;
	int toread = nbytes;
	int *edges = NULL;

	if(type == G3D_IFF_MKID('E','D','G','E'))
	{
		nedges = g3d_read_int16_be(f);
		toread -= 2;
	}
	else
	{
		nedges = g3d_read_int32_be(f);
		toread -= 4;
	}
#if DEBUG > 3
	g_printerr("iob_read_edges: %d edges\n", nedges);
#endif
	edges = g_malloc(nedges * 2 * sizeof(int));
	for(i=0; i<nedges; i++)
	{
		if(type == G3D_IFF_MKID('E','D','G','E'))
		{
			edges[i*2+0] = g3d_read_int16_be(f);
			edges[i*2+1] = g3d_read_int16_be(f);
			toread -= 4;
		}
		else
		{
			edges[i*2+0] = g3d_read_int32_be(f);
			edges[i*2+1] = g3d_read_int32_be(f);
			toread -= 8;
		}
	}
	if(toread != 0)
	{
		g_printerr("error in EDGE chunk (%d bytes to read)\n", toread);
		fseek(f, toread, SEEK_CUR);
	}
	return edges;
}

int iob_read_mat_lists(FILE *f, int nbytes, G3DObject* object, int type)
{
	guint32 nitems;
	guint32 i;
	G3DMaterial *material;
	G3DFace *face;

	if((type & 0xFF) == '2')
		nitems = g3d_read_int32_be(f);
	else
		nitems = g3d_read_int16_be(f);

#if DEBUG > 0
	g_print("IOB: xLST / xLS2: %d items\n", nitems);
#endif
	for(i = 0; i < nitems; i ++)
	{
		/* default material + nth */
		material = g_slist_nth_data(object->materials, i + 1);
		if(material == NULL)
		{
			material = g3d_material_new();
			material->name = g_strdup_printf("per face material #%d", i);
			object->materials = g_slist_append(object->materials, material);

			/* assign to face */
			face = g_slist_nth_data(object->faces, i);
			if(face)
			{
				face->material = material;
			}
		}

		switch(type)
		{
			case G3D_IFF_MKID('C', 'L', 'S', 'T'):
			case G3D_IFF_MKID('C', 'L', 'S', '2'):
				material->r = (gfloat)(g3d_read_int8(f) / 255.0);
				material->g = (gfloat)(g3d_read_int8(f) / 255.0);
				material->b = (gfloat)(g3d_read_int8(f) / 255.0);
				break;

			case G3D_IFF_MKID('R', 'L', 'S', 'T'):
			case G3D_IFF_MKID('R', 'L', 'S', '2'):
				material->specular[0] = (gfloat)(g3d_read_int8(f) / 255.0);
				material->specular[1] = (gfloat)(g3d_read_int8(f) / 255.0);
				material->specular[2] = (gfloat)(g3d_read_int8(f) / 255.0);
				break;

			case G3D_IFF_MKID('T', 'L', 'S', 'T'):
			case G3D_IFF_MKID('T', 'L', 'S', '2'):
				material->a = 1.0 - (
					(gfloat)(g3d_read_int8(f) / 255.0) +
					(gfloat)(g3d_read_int8(f) / 255.0) +
					(gfloat)(g3d_read_int8(f) / 255.0)) / 3.0;
				break;
		}
	}

	/* padding */
	if(nitems % 2)
		g3d_read_int8(f);

	return TRUE;
}

void iob_order_array(int *array, int numelems)
{
	int elem = 0;

	while(elem < numelems)
	{
		int i;
		for(i=elem+1; i<numelems; i++)
		{
			if(array[elem] > array[i])
			{
				int bottle = array[elem];
				array[elem] = array[i];
				array[i] = bottle;
			}
		}
		elem++;
	}
}

int iob_read_faces(FILE *f, int nbytes, G3DObject* object, int *edges, 
									 int type)
{
	int i, nfaces;
	int toread = nbytes;
	int v1, v2, v3;
	int e[3];

	if(type == G3D_IFF_MKID('F','A','C','E'))
	{
		nfaces = g3d_read_int16_be(f);
		toread -= 2;
	}
	else
	{
		nfaces = g3d_read_int32_be(f);
		toread -= 4;
	}

	for(i=0; i<nfaces; i++)
	{
		G3DFace *face = g_new0(G3DFace, 1);
		face->vertex_count = 3;
		face->vertex_indices = g_new0(guint32, 3);
		if(type == G3D_IFF_MKID('F','A','C','E'))
		{
			e[0] = g3d_read_int16_be(f);
			e[1] = g3d_read_int16_be(f);
			e[2] = g3d_read_int16_be(f);
			toread -= 6;
		}
		else
		{
			e[0] = g3d_read_int32_be(f);
			e[1] = g3d_read_int32_be(f);
			e[2] = g3d_read_int32_be(f);
			toread -= 12;
		}
#if DEBUG > 3
		g_printerr("iob_read_faces: #%d (%d|%d)-(%d|%d)-(%d|%d)\n", i,
			edges[e[0]*2], edges[e[0]*2+1],
			edges[e[1]*2], edges[e[1]*2+1],
			edges[e[2]*2], edges[e[2]*2+1]);
#endif
#if 0 /* all mats twoside */
		iob_order_array(e, 3);
#endif
		face->vertex_indices[0] = v1 = edges[e[0]*2+0];
		face->vertex_indices[1] = v2 = edges[e[0]*2+1];

		if((v1!=edges[e[1]*2+0]) && (v2!=edges[e[1]*2+0]))
			v3=edges[e[1]*2+0];
		else if((v1!=edges[e[1]*2+1]) && (v2!=edges[e[1]*2+1]))
			v3=edges[e[1]*2+1];
		else if((v1!=edges[e[2]*2+0]) && (v2!=edges[e[2]*2+0]))
			v3=edges[e[2]*2+0];
		else
			v3 = edges[e[2]*2+1];

		face->vertex_indices[2] = v3;

		face->material = g_slist_nth_data(object->materials, 0);
		object->faces = g_slist_prepend(object->faces, face);

#if DEBUG > 3
		g_printerr("iob_read_faces: face: #%d (%d/%d/%d)\n", i+1,
			face->vertex_indices[0], face->vertex_indices[1],
			face->vertex_indices[2]);
#endif
	}
	if(toread != 0)
	{
		g_printerr("error in FACE chunk (%d bytes to read)\n", toread);
		fseek(f, toread, SEEK_CUR);
		return FALSE;
	}
	return TRUE;
}


