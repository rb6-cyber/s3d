/* $Id: imp_q3o.c,v 1.1.2.3 2006/01/23 17:03:06 dahms Exp $ */

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

#include <g3d/types.h>
#include <g3d/context.h>
#include <g3d/material.h>
#include <g3d/read.h>

/*
 * format description:
 * http://www.quick3d.com/guide/guidec.html
 */

static void q3o_update_face_textures(G3DModel *model, G3DContext *context);
static gboolean q3o_read_mesh(FILE *f, G3DModel *model, guint32 n_textures,
	G3DContext *context);
static gboolean q3o_read_material(FILE *f, G3DModel *model, guint32 index,
	guint32 n_textures);
static gboolean q3o_read_texture(FILE *f, G3DModel *model);
static gboolean q3o_read_scene(FILE *f, G3DContext *context);
static gboolean q3o_read_eof(FILE *f);

gboolean plugin_load_model(G3DContext *context, const gchar *filename,
	G3DModel *model, gpointer user_data)
{
	gchar signature[8], ver_min, ver_maj, id;
	guint32 nmeshes, nmats, ntexs, i;
	FILE *f;

	f = fopen(filename, "rb");
	if(f == NULL)
	{
		g_warning("could not open '%s'", filename);
		return FALSE;
	}

	fread(signature, 1, 8, f);
	if(strncmp(signature, "quick3Ds", 8) && strncmp(signature, "quick3Do", 8))
	{
		g_warning("file '%s' is not a Quick3D file", filename);
		fclose(f);
		return FALSE;
	}

	ver_maj = g3d_read_int8(f);
	ver_min = g3d_read_int8(f);
#if DEBUG > 0
	g_print("Q3O: version %c.%c\n", ver_maj, ver_min);
#endif

	nmeshes = g3d_read_int32_le(f);
	nmats = g3d_read_int32_le(f);
	ntexs = g3d_read_int32_le(f);
#if DEBUG > 0
	g_print("Q3O: %d meshes, %d materials, %d textures\n",
		nmeshes, nmats, ntexs);
#endif

	/* generate (emtpy) materials */
	for(i = 0; i < nmats; i ++)
	{
		G3DMaterial *material = g3d_material_new();
		model->materials = g_slist_append(model->materials, material);
	}

	while((id = g3d_read_int8(f)) != 0)
	{
#if DEBUG > 0
		g_print("Q3O: chunk type 0x%02x @ 0x%08lx\n", id, ftell(f) - 1);
#endif
		switch(id)
		{
			case 'm': /* mesh */
				for(i = 0; i < nmeshes; i ++)
					q3o_read_mesh(f, model, ntexs, context);
				break;

			case 'c': /* material */
				for(i = 0; i < nmats; i ++)
					q3o_read_material(f, model, i, ntexs);
				break;

			case 't': /* texture */
				for(i = 0; i < ntexs; i ++)
					q3o_read_texture(f, model);
				break;

			case 's': /* scene */
				q3o_read_scene(f, context);
				break;

			case 'q': /* EOF signature? */
				q3o_read_eof(f);
				break;

			default:
				g_warning("Q3O: unknown chunk type 0x%02x\n", id);
				fclose(f);
				return TRUE;
				break;
		}
	}

	fclose(f);

	/* update texture images */
	q3o_update_face_textures(model, context);

	return TRUE;
}

gchar *plugin_description(void)
{
	return g_strdup("import plugin for Quick3D objects\n");
}

gchar **plugin_extensions(void)
{
	return g_strsplit("q3o:q3s", ":", 0);
}

/*
 * Q3O specific stuff
 */

static void q3o_update_face_textures(G3DModel *model, G3DContext *context)
{
	GSList *oitem, *fitem;
	G3DObject *object;
	G3DFace *face;

	oitem = model->objects;
	while(oitem)
	{
		object = (G3DObject *)oitem->data;
		fitem = object->faces;
		while(fitem)
		{
			face = (G3DFace *)fitem->data;
			face->tex_image = face->material->tex_image;
			if(face->tex_image && face->tex_image->width)
				face->flags |= G3D_FLAG_FAC_TEXMAP;
			else
			{
				face->tex_vertex_count = 0;
				if(face->tex_vertex_data)
					g_free(face->tex_vertex_data);
			}
			fitem = fitem->next;
		}

		g3d_context_update_interface(context);
		oitem = oitem->next;
	}
}

static G3DImage *q3o_get_texture_nth(G3DModel *model, guint32 n)
{
	gchar number[32];
	G3DImage *image;

	if(model->tex_images == NULL)
		model->tex_images = g_hash_table_new(g_str_hash, g_str_equal);

#if DEBUG > 5
	g_print("Q3O: texture #%d wanted\n", n);
#endif

	sprintf(number, "%d", n);
	image = g_hash_table_lookup(model->tex_images, number);
	if(image)
	{
#if DEBUG > 5
		g_print("Q3O: texture #%d from hash table\n", n);
#endif
		return image;
	}

#if DEBUG > 5
	g_print("Q3O: texture #%d created\n", n);
#endif

	image = g_new0(G3DImage, 1);
	image->tex_scale_u = 1.0;
	image->tex_scale_v = 1.0;
	image->name = g_strdup_printf("would be %d", n + 1);

	g_hash_table_insert(model->tex_images, g_strdup(number), image);

	return image;
}

static gboolean q3o_read_mesh(FILE *f, G3DModel *model, guint32 n_textures,
	G3DContext *context)
{
	guint32 i, j, nfaces, mat, nnormals, ntexco, index, nfaceverts = 0;
	guint16 *faceshapes;
	gfloat *normals;
	GSList *fitem;
	G3DObject *object;
	G3DFace *face;
	G3DMaterial *material;

	object = g_new0(G3DObject, 1);
	object->name = g_strdup("Q3O mesh");
	model->objects = g_slist_append(model->objects, object);

	material = g3d_material_new();
	material->name = g_strdup("fallback material");
	object->materials = g_slist_append(object->materials, material);

	/* vertices */
	object->vertex_count = g3d_read_int32_le(f);
#if DEBUG > 3
	g_print("Q3O: number of vertices: %d\n", object->vertex_count);
#endif
	object->vertex_data = g_new0(gfloat,object->vertex_count * 3);
	for(i = 0; i < object->vertex_count; i ++)
	{
		object->vertex_data[i*3+0] = g3d_read_float_le(f);
		object->vertex_data[i*3+1] = g3d_read_float_le(f);
		object->vertex_data[i*3+2] = g3d_read_float_le(f);

		g3d_context_update_interface(context);
	}

	/* faces */
	nfaces = g3d_read_int32_le(f);
#if DEBUG > 3
    g_print("Q3O: number of faces: %d\n", nfaces);
#endif
	faceshapes = g_new0(guint16, nfaces);
	for(i = 0; i < nfaces; i ++)
	{
		faceshapes[i] = g3d_read_int16_le(f);
		nfaceverts += faceshapes[i];

		g3d_context_update_interface(context);
	}

	for(i = 0; i < nfaces; i ++)
	{
		face = g_new0(G3DFace, 1);
		face->vertex_count = faceshapes[i];
		face->vertex_indices = g_new0(guint32, face->vertex_count);
		for(j = 0; j < face->vertex_count; j ++)
		{
			face->vertex_indices[j] = g3d_read_int32_le(f);
			if(face->vertex_indices[j] >= object->vertex_count)
			{
				g_warning("Q3O: vertex_indices >= vertex_count");
			}

			g3d_context_update_interface(context);
		}
		/* fallback material */
		face->material = (G3DMaterial *)g_slist_nth_data(object->materials, 0);
		g_assert(face->material);

		object->faces = g_slist_append(object->faces, face);

		g3d_context_update_interface(context);
	}

	/* material indices */
	fitem = object->faces;
	for(i = 0; i < nfaces; i ++)
	{
		face = (G3DFace *)fitem->data;
		g_assert(face != NULL);
		mat = g3d_read_int32_le(f);
		face->material = (G3DMaterial*)g_slist_nth_data(model->materials, mat);
		if(face->material == NULL)
		{
			if(mat != -1) g_warning("Q3O: material is NULL (index %d)", mat);
			face->material = g_slist_nth_data(object->materials, 0);
		}

		g3d_context_update_interface(context);
		fitem = fitem->next;
	}

	/* normals */
	nnormals = g3d_read_int32_le(f);
	normals = g_new0(gfloat, nnormals * 3);
#if DEBUG > 3
	g_print("Q3O: number of normals: %d\n", nnormals);
#endif
	for(i = 0; i < nnormals; i ++)
	{
		normals[i * 3 + 0] = g3d_read_float_le(f);
		normals[i * 3 + 1] = g3d_read_float_le(f);
		normals[i * 3 + 2] = g3d_read_float_le(f);

		g3d_context_update_interface(context);
	}

	/* update faces */
	if(object->vertex_count == nnormals)
	{
		fitem = object->faces;
		for(i = 0; i < nfaces; i ++)
		{
			face = (G3DFace *)fitem->data;
			face->normals = g_new0(gfloat, faceshapes[i] * 3);
			face->flags |= G3D_FLAG_FAC_NORMALS;
			for(j = 0; j < faceshapes[i]; j ++)
			{
				face->normals[j * 3 + 0] =
					normals[face->vertex_indices[j] * 3 + 0];
				face->normals[j * 3 + 1] =
					normals[face->vertex_indices[j] * 3 + 1];
				face->normals[j * 3 + 2] =
					normals[face->vertex_indices[j] * 3 + 2];
			}

			g3d_context_update_interface(context);
			fitem = fitem->next;
		}

	}

	/* texture stuff */
	ntexco = g3d_read_int32_le(f);
#if DEBUG > 3
	g_print("Q3O: number of texture coordinates: %d\n", ntexco);
#endif
	if(n_textures > 0)
	{
		object->tex_vertex_count = ntexco;
		object->tex_vertex_data = g_new0(gfloat, 2 * ntexco);

		for(i = 0; i < ntexco; i ++)
		{
			object->tex_vertex_data[i * 2 + 0] = g3d_read_float_le(f);
			object->tex_vertex_data[i * 2 + 1] = g3d_read_float_le(f);
		}

		fitem = object->faces;
		for(i = 0; i < nfaces; i ++)
		{
			face = (G3DFace *)fitem->data;
			face->tex_vertex_count = faceshapes[i];
			face->tex_vertex_data = g_new0(gfloat, faceshapes[i] * 2);
			for(j = 0; j < faceshapes[i]; j ++)
			{
				index = g3d_read_int32_le(f);
				face->tex_vertex_data[j * 2 + 0] =
					object->tex_vertex_data[face->vertex_indices[j] * 2 + 0];
				face->tex_vertex_data[j * 2 + 1] =
					object->tex_vertex_data[face->vertex_indices[j] * 2 + 1];
			}

			g3d_context_update_interface(context);
			fitem = fitem->next;
		}
	}

	/* centerOfMass */
	g3d_read_float_le(f);
	g3d_read_float_le(f);
	g3d_read_float_le(f);

	/* boundingBox */
	g3d_read_float_le(f);
	g3d_read_float_le(f);
	g3d_read_float_le(f);

	g3d_read_float_le(f);
	g3d_read_float_le(f);
	g3d_read_float_le(f);

	/* clean up */
	g_free(faceshapes);
	if(object->tex_vertex_data)
	{
		/* should be in faces */
		g_free(object->tex_vertex_data);
		object->tex_vertex_data = NULL;
	}
	if(normals)
		g_free(normals);

	g3d_context_update_interface(context);

	return TRUE;
}

static gboolean q3o_read_material(FILE *f, G3DModel *model, guint32 index,
	guint32 n_textures)
{
	gchar buffer[2048], *bufp;
	G3DMaterial *material;
	gint32 num;

	material = g_slist_nth_data(model->materials, index);
	memset(buffer, 0, 2048);
	bufp = buffer;
	while((*bufp = g3d_read_int8(f)) != '\0') bufp ++;
	material->name = g_strdup(buffer);
#if DEBUG > 0
	g_print("Q3O: material name: '%s'\n", buffer);
#endif

	/* ambientColor */
	material->r = g3d_read_float_le(f);
	material->g = g3d_read_float_le(f);
	material->b = g3d_read_float_le(f);

	/* diffuseColor */
	material->r = g3d_read_float_le(f);
	material->g = g3d_read_float_le(f);
	material->b = g3d_read_float_le(f);

	/* specularColor */
	material->specular[0] = g3d_read_float_le(f);
	material->specular[1] = g3d_read_float_le(f);
	material->specular[2] = g3d_read_float_le(f);

	/* transparency */
	material->a = g3d_read_float_le(f);
	if(material->a == 0.0) material->a = 1.0;
	if(material->a < 0.1) material->a = 0.1;

	/* texture */
	num = g3d_read_int32_le(f);
#if DEBUG > 4
	g_print("Q3O: material unknown uint32: %d\n", num);
#endif
	if((num != -1) && (num < n_textures))
		material->tex_image = q3o_get_texture_nth(model, num);

	return TRUE;
}

static int q3o_read_texture(FILE *f, G3DModel *model)
{
	G3DImage *image;
	gchar buffer[2048], *bufp;
#if DEBUG > 2
	gchar *ppmname;
#endif
	guint32 width, height, y, x;
	static guint32 index = 0;

	memset(buffer, 0, 2048);
	bufp = buffer;
	while((*bufp = g3d_read_int8(f)) != '\0') bufp ++;

	width = g3d_read_int32_le(f);
	height = g3d_read_int32_le(f);
#if DEBUG > 0
	g_print("Q3O: texture #%d '%s': %dx%d\n", index, buffer, width, height);
#endif

	image = q3o_get_texture_nth(model, index);
	index ++;

	image->name = g_strdup(buffer);
	image->width = width;
	image->height = height;
	image->depth = 32;
	image->pixeldata = g_new0(guint8, width * height * 4);
	image->tex_id = index;

	for(y = 0; y < height; y ++)
		for(x = 0; x < width; x ++)
		{
			image->pixeldata[(y * width + x) * 4 + 0] = g3d_read_int8(f);
			image->pixeldata[(y * width + x) * 4 + 1] = g3d_read_int8(f);
			image->pixeldata[(y * width + x) * 4 + 2] = g3d_read_int8(f);
			image->pixeldata[(y * width + x) * 4 + 3] = 0xFF;
		}

#if DEBUG > 2
	ppmname = g_strdup_printf("/tmp/%s.ppm", image->name);
	g3d_image_dump_ppm(image, ppmname);
	g_free(ppmname);
#endif
	return TRUE;
}

static gboolean q3o_read_scene(FILE *f, G3DContext *context)
{
	gchar buffer[2048], *bufp;
	guint32 bgw, bgh;
	/* position: 3 x float */
	fseek(f, 12, SEEK_CUR);

	/* transformation: matrix */
	fseek(f, 64, SEEK_CUR);

	/* axis: 3 x float */
	fseek(f, 12, SEEK_CUR);

	/* angle: float */
	fseek(f, 4, SEEK_CUR);

	/* eyePosition: 3 x float */
	fseek(f, 12, SEEK_CUR);

	/* eyeRotation: 3 x float */
	fseek(f, 12, SEEK_CUR);

	/* foregroundColor: color */
	fseek(f, 12, SEEK_CUR);

	/* backgroundColor: color */
	g3d_context_set_bgcolor(context,
		g3d_read_float_le(f),
		g3d_read_float_le(f),
		g3d_read_float_le(f),
		1.0);

	/* usingEyeFilter: bool */
	g3d_read_int8(f);

	/* eyeFilterColor: color */
	fseek(f, 12, SEEK_CUR);

	/* eyeFilterAmount: float */
	g3d_read_float_le(f);

	/* lightColor: color */
	fseek(f, 12, SEEK_CUR);

	/* backgroundImageWidth: int */
	bgw = g3d_read_int32_le(f);

	/* backgroundImageHeight: int */
	bgh = g3d_read_int32_le(f);

	if(bgw * bgh)
	{

		/* backgroundFilename */
		memset(buffer, 0, 2048);
		bufp = buffer;
		while((*bufp = g3d_read_int8(f)) != '\0') bufp ++;

#if DEBUG > 0
		g_print("Q3O: scene: background image '%s' (%dx%d)\n",
			buffer, bgw, bgh);
#endif

		/* backgroundImage: pixel[] */
		fseek(f, bgw * bgh * 3, SEEK_CUR);
	}

	/* depthCuing: float */
	g3d_read_float_le(f);

	/* depthCueColor: color */
	fseek(f, 12, SEEK_CUR);

	/* gamma: float */
	g3d_read_float_le(f);

	return FALSE;
}

static int q3o_read_eof(FILE *f)
{
	gchar buffer[8];

	fseek(f, -1, SEEK_CUR);

	if(fread(buffer, 1, 8, f) == 8)
	{
		if(strncmp(buffer, "quick3Ds", 8) == 0) return TRUE;
		g_warning("Q3O: did not get expected EOF marker");
	}
	else
	{
		g_warning("Q3O: premature end of file\n");
	}
	return FALSE;
}

