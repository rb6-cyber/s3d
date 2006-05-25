/* $Id: imp_heightfield.c,v 1.1.2.2 2006/01/23 17:03:06 dahms Exp $ */

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

#include <stdlib.h>
#include <string.h>

#include <g3d/types.h>
#include <g3d/plugins.h>
#include <g3d/material.h>

gboolean plugin_load_model(G3DContext *context, const gchar *filename,
	G3DModel *model, gpointer user_data)
{
	G3DImage *image = g_new0(G3DImage, 1);
	G3DObject *object;
	G3DMaterial *material;
	guint32 x, y, index;

	if(g3d_plugins_load_image(context, filename, image))
	{
		g_free(image);
		return FALSE;
	}

	object = g_new0(G3DObject, 1);
	object->name = g_strdup("height field");
	model->objects = g_slist_append(model->objects, object);

	material = g3d_material_new();
	material->name = g_strdup("default material");
	material->r = 0.2;
	material->g = 1.0;
	material->b = 0.1;
	material->a = 1.0;
	object->materials = g_slist_append(object->materials, material);

	object->vertex_count = image->width * image->height;
	object->vertex_data = g_new0(gfloat, object->vertex_count * 3);

#if DEBUG > 0
	g_printerr("height field loader: image: %dx%dx%d\n",
		image->width, image->height, image->depth);
#endif

	for(y = 0; y < image->height; y ++)
	{
#if DEBUG > 5
		g_print("Heightfield: line: ");
#endif
		for(x=0; x<image->width; x++)
		{
			index = y*image->width + x;

			object->vertex_data[index*3+0] = x;
			object->vertex_data[index*3+1] = y;
			switch(image->depth)
			{
				case 8:
					object->vertex_data[index*3+2] = 0.0 +
						(float)image->pixeldata[index] / 32.0;
					break;
				case 15:
				case 16:
					object->vertex_data[index*3+2] = 0.0 +
						*((short int*)&image->pixeldata[index]);
					break;
				case 24:
				case 32:
					object->vertex_data[index * 3 + 2] = 0.0 +
						image->pixeldata[index * 4] / 32.0;
					break;
				default:
					break;
			}
#if DEBUG > 5
i			g_print(" (%.1f,%.1f,%.1f)",
				object->vertex_data[index*3+0],
				object->vertex_data[index*3+1],
				object->vertex_data[index*3+2]);
#endif
			if((x < (image->width-1)) && (y < (image->height-1)))
			{
				G3DFace *face = g_new0(G3DFace, 1);

				face->material = material;
				face->vertex_count = 3;
				face->vertex_indices = g_new0(guint32, 3);
				face->vertex_indices[0] = index;
				face->vertex_indices[1] = index + 1;
				face->vertex_indices[2] = index + image->width;
				object->faces = g_slist_prepend(object->faces, face);

				face = g_new0(G3DFace, 1);
				face->material = material;
				face->vertex_count = 3;
				face->vertex_indices = g_new0(guint32, 3);
				face->vertex_indices[0] = index + 1;
				face->vertex_indices[1] = index + image->width + 1;
				face->vertex_indices[2] = index + image->width;
				object->faces = g_slist_prepend(object->faces, face);
			}
		} /* for(x) */
#if DEBUG > 4
		g_printerr("loader: line %d ready\n", y+1);
#endif
#if DEBUG > 5
		g_print("\n");
#endif
	} /* for(y) */

	return TRUE;
}

gchar *plugin_description(G3DContext *context)
{
	return g_strdup(
		"plugin to generate height fields from images\n"
		"Author: Markus Dahms\n");
}

gchar **plugin_extensions(G3DContext *context)
{
	return g3d_plugins_get_image_extensions(context);
}

