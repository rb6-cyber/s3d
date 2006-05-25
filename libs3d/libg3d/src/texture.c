/* $Id: texture.c,v 1.1.2.7 2006/01/23 16:38:47 dahms Exp $ */

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
#include <g3d/plugins.h>

static gboolean dump_ppm(G3DImage *image, const gchar *filename);

G3DImage *g3d_texture_load_cached(G3DContext *context, G3DModel *model,
	const gchar *filename)
{
	G3DImage *image;
	gchar *basename, *ppmname, *path;

	/* convert DOS path separator */
	path = g_strdup(filename);
	g_strdelimit(path, "\\", '/');

	/* create hash table if it does not exist yet */
	if(model->tex_images == NULL)
		model->tex_images = g_hash_table_new(g_str_hash, g_str_equal);

	/* if already loaded, return cached image */
	image = g_hash_table_lookup(model->tex_images, path);
	if(image != NULL)
		return image;

	/* create emtpy G3DImage */
	image = g_new0(G3DImage, 1);
	image->tex_scale_u = 1.0;
	image->tex_scale_v = 1.0;
	if(g3d_plugins_load_image(context, path, image))
	{
		g_hash_table_insert(model->tex_images, (gpointer)g_strdup(path),
			image);
	}
	else
	{
		/* try to load file without path */
		basename = g_path_get_basename(path);
		if(g3d_plugins_load_image(context, basename, image))
		{
			g_hash_table_insert(model->tex_images,
				(gpointer)g_strdup(path), image);
		}
		else
		{
			g_free(image);
			image = NULL;
		}
		g_free(basename);
	}

	g_free(path);

	if(0 && image)
	{
		basename = g_path_get_basename(path);
		ppmname = g_strdup_printf("/tmp/%s.ppm", basename);
		dump_ppm(image, ppmname);
		g_free(ppmname);
		g_free(basename);
	}

	return image;
}

gboolean g3d_texture_prepare(G3DImage *texture)
{
	return FALSE;
}

static gboolean dump_ppm(G3DImage *image, const gchar *filename)
{
	FILE *f;
	guint32 x, y;

	f = fopen(filename, "w");
	if(f == NULL)
	{
		g_warning("image: failed to write to '%s'", filename);
		return FALSE;
	}

	fprintf(f, "P3\n# CREATOR: g3dviewer\n%d %d\n%d\n",
		image->width, image->height, 255);

	for(y = 0; y < image->height; y ++)
		for(x = 0; x < image->width; x ++)
			fprintf(f, "%d\n%d\n%d\n",
				image->pixeldata[(y * image->width + x) * 4 + 0],
				image->pixeldata[(y * image->width + x) * 4 + 1],
				image->pixeldata[(y * image->width + x) * 4 + 2]);

	fclose(f);
	return TRUE;
}
