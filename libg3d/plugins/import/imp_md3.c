/* $Id: imp_md3.c,v 1.1.2.7 2006/01/23 17:03:06 dahms Exp $ */

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
#include <math.h>

#ifndef M_PI
#	define M_PI 3.14159265358979323846
#endif

#include <g3d/types.h>
#include <g3d/object.h>
#include <g3d/read.h>
#include <g3d/iff.h>
#include <g3d/material.h>
#include <g3d/texture.h>
#include <g3d/vector.h>

#define MD3_TYPE_MD3 0x01
#define MD3_TYPE_MDC 0x02

gboolean md3_load_skin(G3DContext *context, G3DModel *model,
	const gchar *filename);
gboolean md3_read_tag(FILE *f, G3DContext *context, G3DModel *model);
gboolean md3_read_mesh(FILE *f, G3DContext *context, G3DModel *model);

gboolean plugin_load_model(G3DContext *context, const gchar *filename,
	G3DModel *model, gpointer user_data)
{
	FILE *f;
	guint32 magic, version, nboneframes, ntags, nmeshes, nskins;
	guint32 off_bfs, off_tags, off_meshes, filesize, i, flags;

	f = fopen(filename, "rb");
	if(f == NULL)
	{
		g_warning("MD3: failed to open '%s'", filename);
		return FALSE;
	}

	magic = g3d_read_int32_be(f);
	if((magic != G3D_IFF_MKID('I', 'D', 'P', '3')) &&
		(magic != G3D_IFF_MKID('I', 'D', 'P', 'C')))
	{
		g_warning("MD3: %s is not a valid md3 file", filename);
		fclose(f);
		return FALSE;
	}

	version = g3d_read_int32_le(f);
	fseek(f, 64, SEEK_CUR);

	flags = g3d_read_int32_le(f);
	nboneframes = g3d_read_int32_le(f);
	ntags = g3d_read_int32_le(f);
	nmeshes = g3d_read_int32_le(f);
	nskins = g3d_read_int32_le(f);
	off_bfs = g3d_read_int32_le(f);
	off_tags = g3d_read_int32_le(f);
	off_meshes = g3d_read_int32_le(f);
	filesize = g3d_read_int32_le(f);

	/* try to load skin */
	md3_load_skin(context, model, filename);

	g_print("MD3: version: %u, file size: %u bytes\n", version, filesize);
	g_print("MD3: tags @ 0x%08x, meshes @ 0x%08x\n", off_tags, off_meshes);

	fseek(f, off_tags, SEEK_SET);
	if(magic == G3D_IFF_MKID('I', 'D', 'P', '3'))
	for(i = 0; i < nboneframes * ntags; i ++)
		md3_read_tag(f, context, model);

	/* read meshes */
	fseek(f, off_meshes, SEEK_SET);
	for(i = 0; i < nmeshes; i ++)
		md3_read_mesh(f, context, model);

	return TRUE;
}

gchar *plugin_description(G3DContext *context)
{
	return g_strdup("Quake 3 model loading plugin\n");
}

gchar **plugin_extensions(G3DContext *context)
{
	return g_strsplit("md3:mdc", ":", 0);
}

/*
 * MD3 specific
 */

gboolean md3_load_skin(G3DContext *context, G3DModel *model,
	const gchar *filename)
{
	gchar *basename, *skinname, **parts;
	gchar line[256];
	FILE *f;
	G3DMaterial *material;

	basename = g_path_get_basename(filename);
	skinname = g_strdup_printf("%.*s_default.skin",
		strlen(basename) - 4, basename);

	g_print("MD3: trying to open skin file %s\n", skinname);

	f = fopen(skinname, "r");

	g_free(basename);
	g_free(skinname);

	/* no skin */
	if(f == NULL)
		return FALSE;

	while(fgets(line, 255, f) != NULL)
	{
		parts = g_strsplit(line, ",", 2);
		if(parts[0] && parts[1])
		{
			g_strchomp(parts[1]);
			if(strlen(parts[1]) > 0)
			{
				g_print("MD3: skin texture for %s: %s\n",
					parts[0], parts[1]);

				material = g3d_material_new();
				material->name = g_strdup(parts[0]);
				material->tex_image = g3d_texture_load_cached(context, model,
					parts[1]);

				model->materials = g_slist_append(model->materials,
					material);
			}
		}
		g_strfreev(parts);
	}

	fclose(f);

	return TRUE;
}

gboolean md3_read_tag(FILE *f, G3DContext *context, G3DModel *model)
{
	gchar name[65];

	fread(name, 1, 64, f);
	name[64] = '\0';

	g_print("MD3: tag: %s\n", name);

	/* position */
	g3d_read_float_le(f);
	g3d_read_float_le(f);
	g3d_read_float_le(f);

	/* rotation */
	g3d_read_float_le(f);
	g3d_read_float_le(f);
	g3d_read_float_le(f);

	g3d_read_float_le(f);
	g3d_read_float_le(f);
	g3d_read_float_le(f);

	g3d_read_float_le(f);
	g3d_read_float_le(f);
	g3d_read_float_le(f);

	return TRUE;
}

gboolean md3_read_mesh(FILE *f, G3DContext *context, G3DModel *model)
{
	G3DObject *object;
	G3DImage *image = NULL;
	G3DMaterial *material, *mat;
	G3DFace *face;
	GSList *mitem;
	guint32 magic, i, j;
	guint8 type = 0, r, s;
	gfloat rho, sigma, *normals;
	gchar name[64], *strp;
	guint32 nmeshframe, nskin, nvertex, ntris, mlength, flags;
	guint32 off_tris, off_texvec, off_vertex, off_start, off_skins;
	static guint32 tex_id = 1;

	off_start = ftell(f);

	magic = g3d_read_int32_be(f);

	if(magic == G3D_IFF_MKID('I', 'D', 'P', '3'))
		type = MD3_TYPE_MD3;
	else /* if(magic == 0x07000000)*/
		type = MD3_TYPE_MDC;
#if 0
	else
	{
		g_warning("MD3: mesh magic unknown (%02x%02x%02x%02x)\n",
			(magic >> 24) & 0xFF,
			(magic >> 16) & 0xFF,
			(magic >> 8) & 0xFF,
			magic & 0xFF);
		return FALSE;
	}
#endif

	object = g_new0(G3DObject, 1);

	/* read name */
	fread(name, 1, 64, f);
	object->name = g_strndup(name, 64);

	flags = g3d_read_int32_le(f);

	if(type == MD3_TYPE_MD3)
	{
		nmeshframe = g3d_read_int32_le(f);
		nskin = g3d_read_int32_le(f);
	}
	else if(type == MD3_TYPE_MDC)
	{
		g3d_read_int32_le(f); /* ncompframes */
		g3d_read_int32_le(f); /* nbaseframes */
		g3d_read_int32_le(f); /* nshaders */
	}

	nvertex = g3d_read_int32_le(f);
	ntris = g3d_read_int32_le(f);

	off_tris = g3d_read_int32_le(f);
	off_skins = g3d_read_int32_le(f);

	off_texvec = g3d_read_int32_le(f);
	off_vertex = g3d_read_int32_le(f);

	if(type == MD3_TYPE_MDC)
	{
		g3d_read_int32_le(f); /* off_compvert */
		g3d_read_int32_le(f); /* off_fbasef */
		g3d_read_int32_le(f); /* off_fcompf */
	}

	mlength = g3d_read_int32_le(f);

	if((nvertex == 0) || (ntris == 0))
	{
		g_warning("MD3: %u vertices, %u triangles", nvertex, ntris);
		fseek(f, off_start + mlength, SEEK_SET);
		return FALSE;
	}

	/* default material */
	material = g3d_material_new();
	material->name = g_strdup("default material");
	object->materials = g_slist_append(object->materials, material);

	/* skins */
	fseek(f, off_start + off_skins, SEEK_SET);
	fread(name, 1, 64, f);
	g_print("MD3: skin name: %s\n", name);

	/* read texture image */
	if(strlen(name) > 0)
	{
		image = g3d_texture_load_cached(context, model, name);
		if(image == NULL)
		{
			/* try jpeg */
			strp = strrchr(name, '.');
			if(strp)
			{
				strcpy(strp, ".jpg");
				image = g3d_texture_load_cached(context, model, name);
			}
		}
	}

	if(image == NULL)
	{
		mitem = model->materials;
		while(mitem)
		{
			mat = (G3DMaterial *)mitem->data;
			if(strcmp(mat->name, object->name) == 0)
			{
				image = mat->tex_image;
				break;
			}
			mitem = mitem->next;
		}
	}

	if(image && (image->tex_id == 0))
	{
		image->tex_id = tex_id;
		tex_id ++;
	}

	/* read vertex data */
	fseek(f, off_start + off_vertex, SEEK_SET);
	object->vertex_count = nvertex;
	object->vertex_data = g_new0(gfloat, nvertex * 3);
	normals = g_new0(gfloat, nvertex * 3);
	for(i = 0; i < nvertex; i ++)
	{
		gint16 d;

		d = g3d_read_int16_le(f);
		object->vertex_data[i * 3 + 0] = d;
		d = g3d_read_int16_le(f);
		object->vertex_data[i * 3 + 1] = d;
		d = g3d_read_int16_le(f);
		object->vertex_data[i * 3 + 2] = d;

		/* compressed normal */
		/* FIXME: the normals don't look right... */
		r = g3d_read_int8(f); /* rho */
		s = g3d_read_int8(f); /* sigma */
		rho = r * 2 * M_PI / 256.0;
		sigma = s * 2 * M_PI / 256.0;

		normals[i * 3 + 0] = - cos(sigma) * sin(rho);
		normals[i * 3 + 1] = - cos(sigma) * sin(rho);
		normals[i * 3 + 2] = - cos(rho);

		g3d_vector_unify(
			&(normals[i * 3 + 0]),
			&(normals[i * 3 + 1]),
			&(normals[i * 3 + 2]));
	}

	/* read texture vertex data */
	fseek(f, off_start + off_texvec, SEEK_SET);
	object->tex_vertex_data = g_new0(gfloat, nvertex * 2);
	for(i = 0; i < nvertex; i ++)
	{
		object->tex_vertex_data[i * 2 + 0] = g3d_read_float_le(f);
		object->tex_vertex_data[i * 2 + 1] = g3d_read_float_le(f);
	}

	/* read triangles */
	fseek(f, off_start + off_tris, SEEK_SET);
	for(i = 0; i < ntris; i ++)
	{
		face = g_new0(G3DFace, 1);
		face->vertex_count = 3;
		face->vertex_indices = g_new0(guint32, 3);
		face->material = material;

		face->flags |= G3D_FLAG_FAC_NORMALS;
		face->normals = g_new0(gfloat, 3 * 3);

		face->tex_image = image;
		if(face->tex_image)
		{
			face->tex_vertex_data = g_new0(gfloat, 3 * 2);
			face->flags |= G3D_FLAG_FAC_TEXMAP;
		}

		for(j = 0; j < 3; j ++)
		{
			face->vertex_indices[j] = g3d_read_int32_le(f);

			/* copy normals */
			face->normals[j * 3 + 0] =
				normals[face->vertex_indices[j] * 3 + 0];
			face->normals[j * 3 + 1] =
				normals[face->vertex_indices[j] * 3 + 1];
			face->normals[j * 3 + 2] =
				normals[face->vertex_indices[j] * 3 + 2];

			/* texture stuff */
			if(face->tex_image)
			{
				face->tex_vertex_data[j * 2 + 0] =
					object->tex_vertex_data[face->vertex_indices[j] * 2 + 0];
				face->tex_vertex_data[j * 2 + 1] =
					object->tex_vertex_data[face->vertex_indices[j] * 2 + 1];
			}
		}

		object->faces = g_slist_append(object->faces, face);
	}

	/* free unused data */
	if(object->tex_vertex_data)
	{
		g_free(object->tex_vertex_data);
		object->tex_vertex_data = NULL;
	}
	if(normals)
		g_free(normals);

	model->objects = g_slist_append(model->objects, object);

	fseek(f, off_start + mlength, SEEK_SET);

	return TRUE;
}

