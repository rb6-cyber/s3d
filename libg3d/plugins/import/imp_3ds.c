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

int x3ds_container(FILE *f, guint32 nb, G3DContext *context, G3DModel *model,
	gpointer pobject, gint32 level, gint32 parent);
int x3ds_read_cstr(FILE *f, char *string);
int x3ds_read_pointarray(FILE *f, int nb, G3DObject *object, int level);
int x3ds_read_facearray(FILE *f, int nb, G3DObject *object);
int x3ds_read_meshmatgrp(FILE *f, int nb, G3DModel *model, G3DObject *object,
	int level);
void x3ds_debug(int level, char *format, ...);
G3DObject *x3ds_newobject(G3DModel *model, const char *name);

/* FIXME: replace */
float x3ds_global_scale = 1.0;

/*****************************************************************************/
/* plugin interface                                                          */
/*****************************************************************************/

gboolean plugin_load_model(G3DContext *context, const gchar *filename,
	G3DModel *model, gpointer plugin_data)
{
	FILE *f;
	gint32 nbytes, magic;

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

	x3ds_container(f, nbytes, context, model, NULL, 1, magic);

	fclose(f);
#if DEBUG > 0
	g_printerr("imp_3ds.c: %s successfully loaded\n", filename);
#endif
	return TRUE;
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

int x3ds_container(FILE *f, guint32 nb, G3DContext *context, G3DModel *model,
	gpointer pobject, gint32 level, gint32 parent)
{
	gint32 chunk_id, chunk_len, nbytes, toread;
	char name[2048];
	G3DObject *new_object;

	nbytes = nb;
	while(nbytes > 0)
	{
		chunk_id  = g3d_read_int16_le(f);
		chunk_len = g3d_read_int32_le(f);
		nbytes -= 6;
		chunk_len -= 6;
		toread = chunk_len;
		switch(chunk_id)
		{
			case 0x0002: { /* M3D version */
				int version = g3d_read_int32_le(f);
				x3ds_debug(level, "[%4.4XH] M3D Version: %d.%d (%d byte)\n",
					 chunk_id, version & 0xFF, version >> 16, toread);
				toread -= 4; }
				break;
			case 0x0011: { /* color 24 */
				int r,g,b;
				r = g3d_read_int8(f);
				g = g3d_read_int8(f);
				b = g3d_read_int8(f);
				x3ds_debug(level,
					"[%4.4XH] color (24 bit): #%.2X%.2X%.2X (%d byte)\n",
					chunk_id, r,g,b, toread);
				if((parent == 0xA010) || /* ambient color */
					 (parent == 0xA020))   /* diffuse */
				{
					((G3DMaterial*)pobject)->r = (float)r / 255.0;
					((G3DMaterial*)pobject)->g = (float)g / 255.0;
					((G3DMaterial*)pobject)->b = (float)b / 255.0;
				}
				else if(parent == 0xA030) /* specular */
				{
					((G3DMaterial*)pobject)->specular[0] = (float)r / 255.0;
					((G3DMaterial*)pobject)->specular[1] = (float)g / 255.0;
					((G3DMaterial*)pobject)->specular[2] = (float)b / 255.0;
					((G3DMaterial*)pobject)->specular[3] = 0.25;
				}
				else g_printerr("*** unhandled ***\n");
				toread -= 3; }
				break;
			case 0x0030: { /* short percentage */
				int percent = g3d_read_int16_le(f);
				x3ds_debug(level, "[%4.4XH] percentage: %d%% (%d byte)\n",
					chunk_id, percent, toread);
				toread -= 2;
				if(parent == 0xA040) /* shininess */
				{
					((G3DMaterial*)pobject)->shininess =
						(gfloat)percent / 100.0;
				}
				else if(parent == 0xA050) /* transparency */
				{
					((G3DMaterial*)pobject)->a =
						1.0 - ((gfloat)percent / 100.0);
				}
				else g_printerr("*** unhandled ***\n");
				} break;

			case 0x0031: { /* float percentage */
				float fpercent = g3d_read_float_le(f);
				x3ds_debug(level, "[%4.4XH] float percentage: %f (%d byte)\n",
					chunk_id, fpercent, toread);
				toread -= 4;
				if(parent == 0xA040) /* shininess */
				{
					((G3DMaterial*)pobject)->shininess = fpercent;
				}
				else g_printerr("*** unhandled ***\n");
				} break;

			case 0x0100: /* global scale */
				x3ds_global_scale = g3d_read_float_le(f);
				x3ds_debug(level, "[%4.4XH] master scale: %f (%d byte)\n",
									 chunk_id, x3ds_global_scale, toread);
				toread -= 4;
				break;
			case 0x3D3D: /* mesh */
				x3ds_debug(level, "[%4.4XH] 3DS mesh object (%d byte)\n",
									 chunk_id, toread);
				x3ds_container(f, toread, context, model, NULL, level + 1,
					chunk_id);
				toread = 0;
				break;
			case 0x3D3E: { /* mesh version */
				int minor, major;
				major = g3d_read_int16_le(f);
				minor = g3d_read_int16_le(f);
				x3ds_debug(level, "[%4.4XH] mesh version %d.%d (%d byte)\n",
					chunk_id, major, minor, toread);
				toread -= 4; }
				break;
			case 0x4000: /* named object */
				toread -= x3ds_read_cstr(f, name);
				new_object = x3ds_newobject(model, name);
				x3ds_debug(level,
					"[%4.4XH] named object: %s (%d byte)\n", chunk_id,
					 name, toread);
				x3ds_container(f, toread, context, model, new_object,
					level + 1, chunk_id);
				toread = 0;
				break;
			case 0x4100: /* named triangle object */
				x3ds_debug(level,
					"[%4.4XH] named triangle object: %s (%d byte)\n",
					chunk_id, "", chunk_len);
				x3ds_container(f, toread, context, model, pobject, level + 1,
					chunk_id);
				toread = 0;
				break;
			case 0x4110: /* point array */
				toread = x3ds_read_pointarray(f, toread, (G3DObject*)pobject,
					level);
				break;
			case 0x4111: /* point flag array */
				x3ds_debug(level, "[%4.4XH] point flag array (%d byte)\n",
					chunk_id, toread);
				fseek(f, toread, SEEK_CUR);
				toread = 0;
				break;
			case 0x4120: /* face array */
				x3ds_debug(level, "[%4.4XH] face array (%d byte)\n",
									 chunk_id, toread);
				toread = x3ds_read_facearray(f, toread, (G3DObject*)pobject);
				if(toread != 0)
				{
					x3ds_container(f, toread, context, model, pobject,
						level + 1, 0x4120);
					toread = 0;
				}
				break;
			case 0x4130: /* mesh mat group */
				toread = x3ds_read_meshmatgrp(f, toread, model,
					(G3DObject*)pobject, level);
				break;
			case 0x4140: { /* texture vertices */
				G3DObject *object = (G3DObject*)pobject;
				int i, bytes = toread;
				object->tex_vertex_count = g3d_read_int16_le(f);
				toread -= 2;
				object->tex_vertex_data =
					g_new0(gfloat, object->tex_vertex_count * 2);
				for(i=0; i<object->tex_vertex_count; i++)
				{
					object->tex_vertex_data[i*2+0] = g3d_read_float_le(f);
					object->tex_vertex_data[i*2+1] = g3d_read_float_le(f);
					toread -= 8;
				}
				x3ds_debug(level,
					"[%4.4XH] texture vertices: %d verts (%d byte)\n",
					chunk_id, object->tex_vertex_count, bytes);
				} break;
			case 0x4150: /* smooth group */
				x3ds_debug(level, "[%4.4XH] smooth group (%d byte)\n",
					chunk_id, toread);
				fseek(f, toread, SEEK_CUR);
				toread = 0;
				break;
			case 0x4160: { /* mesh matrix / local axis */
				gfloat matrix[16];
				int w, h;
				x3ds_debug(level, "[%4.4XH] mesh matrix (%d byte)\n",
					chunk_id, toread);
				for(w = 0; w < 4; w ++)
				{
#if DEBUG > 0
					g_print("MATRIX:");
#endif
					for(h = 0; h < 3; h ++)
					{
						matrix[h * 4 + w] = g3d_read_float_le(f);
#if DEBUG > 0
						g_print(" %+2.2f", matrix[h * 4 + w]);
#endif
					}
#if DEBUG > 0
					g_print("\n");
#endif
				}

				matrix[12] = matrix[13] = matrix[14] = 0.0;
				matrix[15] = 1.0;

#if 0
				for(i = 0; i < object->vertex_count; i ++)
					g3d_vector_transform(
						&(object->vertices[i * 3 + 0]),
						&(object->vertices[i * 3 + 1]),
						&(object->vertices[i * 3 + 2]),
						matrix);
#endif
				toread -= 48;
				} break;

			case 0x4165: /* mesh color */
				x3ds_debug(level, "[%4.4XH] mesh color %d (%d byte)\n",
					chunk_id, g3d_read_int8(f), toread);
				toread -= 1;
				fseek(f, toread, SEEK_CUR);
				toread = 0;
				break;

			case 0x4170: /* texture info */
				x3ds_debug(level, "[%4.4XH] texture info (%d byte)\n",
					chunk_id, toread);
				g3d_read_int16_le(f); /* map_type */
				toread -= 2;
				g3d_read_float_le(f); /* x_tiling */
				g3d_read_float_le(f); /* y_tiling */
				toread -= 8;
				g3d_read_float_le(f); /* icon_x */
				g3d_read_float_le(f); /* icon_y */
				g3d_read_float_le(f); /* icon_z */
				toread -= 12;
				fseek(f, 4 * 12, SEEK_CUR); /* matrix (4 x 3) */
				toread -= 4 * 12;
				g3d_read_float_le(f); /* scaling */
				g3d_read_float_le(f); /* plan_icon_w */
				g3d_read_float_le(f); /* plan_icon_h */
				g3d_read_float_le(f); /* cyl_icon_h */
				toread -= 16;
				break;

			case 0xA000: /* material name */
				toread -= x3ds_read_cstr(f, name);
				if(pobject != NULL)
				{
					((G3DMaterial*)pobject)->name = g_strdup(name);
				}
				else
				{
					g_printerr("material object = NULL :(\n");
				}
				x3ds_debug(level, "[%4.4XH] material name: %s (%d byte)\n",
									 chunk_id, name, chunk_len);
				break;

			case 0xA010: /* ambient color */
			case 0xA020: /* diffuse color */
			case 0xA030: /* specular color */
				x3ds_debug(level, "[%4.4XH] material color (%d byte)\n",
					chunk_id, toread);
				x3ds_container(f, toread, context, model, pobject, level + 1,
					chunk_id);
				toread = 0;
				break;
			case 0xA040: /* shininess */
				x3ds_debug(level, "[%4.4XH] shininess (%d byte)\n", chunk_id,
									 toread);
				x3ds_container(f, toread, context, model, pobject, level + 1,
					chunk_id);
				toread = 0;
				break;
			case 0xA050: /* transparency */
				x3ds_debug(level, "[%4.4XH] transparency (%d byte)\n", chunk_id,
									 toread);
				x3ds_container(f, toread, context, model, pobject, level + 1,
					chunk_id);
				toread = 0;
				break;
			case 0xA052: /* XPFALL? */
				x3ds_debug(level, "[%4.4XH] fallthrough?? (%d byte)\n", chunk_id,
									 toread);
				x3ds_container(f, toread, context, model, pobject, level + 1,
					chunk_id);
				toread = 0;
				break;
			case 0xA053: /* REFBLUR? */
				x3ds_debug(level, "[%4.4XH] blur?? (%d byte)\n", chunk_id,
									 toread);
				x3ds_container(f, toread, context, model, pobject, level + 1,
					chunk_id);
				toread = 0;
				break;
			case 0xA081:
				x3ds_debug(level, "[%4.4XH] mat two side (%d byte)\n", chunk_id,
									 toread);
				((G3DMaterial*)pobject)->flags |= G3D_FLAG_MAT_TWOSIDE;
				break;
			case 0xA100: { /* shading */
				int shade = g3d_read_int16_le(f);
				x3ds_debug(level, "[%4.4XH] shading: %d (%d byte)\n", chunk_id,
									 shade, toread);
				toread -= 2; }
				break;
			case 0xA200: /* texture map */
				x3ds_debug(level, "[%4.4XH] texture map (%d byte)\n", chunk_id,
									 toread);
				((G3DMaterial*)pobject)->flags |= G3D_FLAG_FAC_TEXMAP;
				x3ds_container(f, toread, context, model, pobject, level + 1,
					chunk_id);
				toread = 0;
				break;
			case 0xA300: /* map name */
				if(parent == 0xA200)
				{
					G3DMaterial *material = (G3DMaterial*)pobject;
					toread -= x3ds_read_cstr(f, name);
					x3ds_debug(level, "[%4.4XH] map name \"%s\" (%d byte)\n",
						chunk_id, name, toread);
					material->tex_image = g3d_texture_load_cached(context,
						model, name);
					if(material->tex_image)
					{
						/* FIXME: better way to generate unique texture ids */
						material->tex_image->tex_id = g_random_int();
						/*
						g3d_texture_prepare(material->texture);
						*/
					}
				}
				break;

			case 0xA351: /* map tiling */
				x3ds_debug(level, "[%4.4XH] texture map tiling (%d byte)\n",
					chunk_id, toread);
				g3d_read_int16_le(f);
				toread -= 2;
				break;

			case 0xA352: /* old map blurring */
				x3ds_debug(level,
					"[%4.4XH] texture map blurring (old) (%d byte)\n",
					chunk_id, toread);
				break;

			case 0xA353: /* map blurring */
				x3ds_debug(level, "[%4.4XH] texture map blurring (%d byte)\n",
					chunk_id, toread);
				g3d_read_float_le(f);
				toread -= 4;
				break;

			case 0xAFFF: { /* material entry */
				G3DMaterial *material = g3d_material_new();
				x3ds_debug(level, "[%4.4XH] material entry (%d byte)\n",
					chunk_id, toread);
				model->materials = g_slist_append(model->materials, material);
#if DEBUG > 3
				g_printerr("DEBUG: material created & appended\n");
#endif
				x3ds_container(f, toread, context, model, material, level + 1,
					chunk_id);
				toread = 0; }
				break;
			default: /* unknown chunk type */
				x3ds_debug(level, "[%4.4XH] unknown (%d byte)\n",
					chunk_id, toread);
				fseek(f, toread, SEEK_CUR);
				toread = 0;
		}
		if(toread != 0)
		{
			g_printerr("WARNING: toread != 0 (%d), chunk_id: %4.4X\n", 
								 toread, chunk_id);
			fseek(f, toread, SEEK_CUR);
		}
		nbytes -= chunk_len;
	}
	return TRUE;
}

int x3ds_read_cstr(FILE *f, char *string)
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

int x3ds_read_pointarray(FILE *f, int nb, G3DObject *object, int level)
{
	int i, nbytes = nb;

	object->vertex_count = g3d_read_int16_le(f);

	x3ds_debug(level, "[%4.4XH] point array: %d verts (%d bytes)\n",
			0x4110, object->vertex_count, nbytes);

	object->vertex_data = g_new0(gfloat, object->vertex_count * 3);
	nbytes -= 2;
	nbytes -= (object->vertex_count * 3 * 4);
	for(i=0; i<object->vertex_count; i++)
	{
		object->vertex_data[i*3+0] = g3d_read_float_le(f) * x3ds_global_scale;
		object->vertex_data[i*3+1] = g3d_read_float_le(f) * x3ds_global_scale;
		object->vertex_data[i*3+2] = g3d_read_float_le(f) * x3ds_global_scale;
	}
	return nbytes;
}

int x3ds_read_facearray(FILE *f, int nb, G3DObject *object)
{
	int nbytes = nb;
	int i, flags, nfaces = g3d_read_int16_le(f);
#if 1
	int p1=-1, p2=-1;
#endif
	nbytes -= 2;
#if DEBUG > 3
	g_printerr("faces: %d\n", nfaces);
#endif
	for(i=0; i<nfaces; i++)
	{
		G3DFace *face = g_malloc0(sizeof(G3DFace));

		face->vertex_count = 3;
		face->vertex_indices = g_malloc(3 * sizeof(int));
		face->vertex_indices[0] = g3d_read_int16_le(f);
		face->vertex_indices[1] = g3d_read_int16_le(f);
		face->vertex_indices[2] = g3d_read_int16_le(f);
		flags = g3d_read_int16_le(f); /* flags */

#if 1
		if((p1 == face->vertex_indices[0]) && (p2 == face->vertex_indices[1]))
		{
			int bottle = face->vertex_indices[0];
			face->vertex_indices[0] = face->vertex_indices[2];
			face->vertex_indices[2] = bottle;
		}
#endif
#if DEBUG > 3
		g_printerr("x3ds_read_facearray: (%d|%d|%d) flags: 0x%4.4X\n",
			face->vertex_indices[0], face->vertex_indices[1],
			face->vertex_indices[2], flags);
#endif
#if 1
		p1 = face->vertex_indices[0];
		p2 = face->vertex_indices[1];
#endif
		nbytes -= 8;
		face->material = g_slist_nth_data(object->materials, 0);

		object->faces = g_slist_append(object->faces, face);
	}
	return nbytes;
}

int x3ds_read_meshmatgrp(FILE *f, int nb, G3DModel *model, G3DObject *object,
	int level)
{
	int nbytes = nb;
	char name[2048];
	int nfaces, i, j, facenum;
	G3DMaterial *material = NULL;
	GSList *mlist;

	nbytes -= x3ds_read_cstr(f, name);
	x3ds_debug(level, "[%4.4XH] mesh mat group: %s\n", 0x4130, name);

	mlist = model->materials;
	while(mlist != NULL)
	{
		G3DMaterial *mat = (G3DMaterial*)mlist->data;
		if(strcmp(mat->name, name) == 0)
		{
			material = mat;
			break;
		}
		mlist = mlist->next;
	}

	nfaces = g3d_read_int16_le(f);
	nbytes -= 2;
	for(i=0; i<nfaces; i++)
	{
		facenum = g3d_read_int16_le(f);
		nbytes -= 2;
		if(material != NULL)
		{
			G3DFace *face = (G3DFace*)g_slist_nth_data(object->faces, facenum);
			if(face != NULL) face->material = material;

			if(face->material->tex_image && object->tex_vertex_data)
			{
#if DEBUG > 5
				g_print("3ds: textured face\n");
#endif
				face->flags |= G3D_FLAG_FAC_TEXMAP;
				face->tex_image = face->material->tex_image;
				face->tex_vertex_count = 3;
				face->tex_vertex_data = g_new0(gfloat, 6);
				for(j = 0; j < 3; j ++)
				{
					face->tex_vertex_data[j * 2 + 0] =
						object->tex_vertex_data[
							face->vertex_indices[j] * 2 + 0];
					face->tex_vertex_data[j * 2 + 1] =
						object->tex_vertex_data[
							face->vertex_indices[j] * 2 + 1];
				}
			} /* textured face */

		}
	}
	return nbytes;
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

