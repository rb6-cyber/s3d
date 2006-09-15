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

G3DImage *g3d_texture_load(G3DContext *context, const gchar *filename)
{
	G3DImage *image;
	gchar *basename, *path, *casedup, *caseddown, *realfile = NULL;

	/* convert DOS path separator */
	path = g_strdup(filename);
	g_strdelimit(path, "\\", '/');

	if(g_file_test(path, G_FILE_TEST_EXISTS))
	{
		realfile = g_strdup(path);
	}
	else
	{
		basename = g_path_get_basename(path);
		if(g_file_test(basename, G_FILE_TEST_EXISTS))
		{
			realfile = g_strdup(basename);
		}
		else
		{
			casedup = g_ascii_strup(basename, -1);
			if(g_file_test(casedup, G_FILE_TEST_EXISTS))
			{
				realfile = g_strdup(casedup);
			}
			else
			{
				caseddown = g_ascii_strdown(basename, -1);
				if(g_file_test(caseddown, G_FILE_TEST_EXISTS))
				{
					realfile = g_strdup(caseddown);
				}
				g_free(caseddown);
			}
			g_free(casedup);
		}
		g_free(basename);
	}
	g_free(path);

	if(realfile == NULL)
	{
		g_printerr("failed to find a file matching '%s'\n", filename);
		return NULL;
	}

	/* create emtpy G3DImage */
	image = g_new0(G3DImage, 1);
	image->tex_scale_u = 1.0;
	image->tex_scale_v = 1.0;

	if(g3d_plugins_load_image(context, realfile, image))
	{
		g_free(realfile);
		return image;
	}

	g_free(image);
	g_free(realfile);

	return NULL;
}

G3DImage *g3d_texture_load_cached(G3DContext *context, G3DModel *model,
	const gchar *filename)
{
	G3DImage *image;
	gchar *basename, *ppmname;

	/* create hash table if it does not exist yet */
	if(model->tex_images == NULL)
		model->tex_images = g_hash_table_new(g_str_hash, g_str_equal);

	/* if already loaded, return cached image */
	image = g_hash_table_lookup(model->tex_images, filename);
	if(image != NULL)
		return image;

	image = g3d_texture_load(context, filename);
	if(image != NULL)
	{
		g_hash_table_insert(model->tex_images, (gpointer)g_strdup(filename),
			image);
	}

	if(0 && image)
	{
		basename = g_path_get_basename(filename);
		ppmname = g_strdup_printf("/tmp/%s.ppm", basename);
		dump_ppm(image, ppmname);
		g_free(ppmname);
		g_free(basename);
	}

	return image;
}

void g3d_texture_free(G3DImage *texture)
{
	if(texture->name) g_free(texture->name);
	if(texture->pixeldata) g_free(texture->pixeldata);
	g_free(texture);
}

gboolean g3d_texture_prepare(G3DImage *texture)
{
	return FALSE;
}

gboolean g3d_texture_flip_y(G3DImage *texture)
{
	guint8 *newpixel;
	gint32 y;

	g_return_val_if_fail(texture != NULL, FALSE);

	newpixel = g_new0(guint8, texture->width * texture->height * 4);

	for(y = 0; y < texture->height; y ++)
	{
		memcpy(
			newpixel + (y * texture->width * 4),
			texture->pixeldata + (
				(texture->height - y - 1) * texture->width * 4),
			texture->width * 4);
	}

	g_free(texture->pixeldata);
	texture->pixeldata = newpixel;

	return TRUE;
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

G3DImage *g3d_texture_merge_alpha(G3DImage *image, G3DImage *aimage)
{
	G3DImage *texture;
	gint32 x, y;
	gboolean negative;

	g_return_val_if_fail(aimage != NULL, NULL);

	if(image && (
			(image->width != aimage->width) ||
			(image->height != aimage->height)))
	{
		/* size doesn't match, don't do something */
		return image;
	}

	if(image)
	{
		texture = image;
	}
	else
	{
		texture = g_new0(G3DImage, 1);
		texture->tex_scale_u = 1.0;
		texture->tex_scale_v = 1.0;
		texture->width = aimage->width;
		texture->height = aimage->height;
		texture->depth = 4;
		texture->pixeldata = g_malloc(texture->width * texture->height * 4);
	}

	/* negative map? */
	/* FIXME: better solution? */
	if(aimage->pixeldata[0] == 0)
		negative = TRUE;
	else
		negative = FALSE;

	for(y = 0; y < texture->height; y ++)
	{
		for(x = 0; x < texture->width; x ++)
		{
			texture->pixeldata[(y * image->width + x) * 4 + 3] = (negative ?
				255 - aimage->pixeldata[(y * image->width + x) * 4 + 0] :
				aimage->pixeldata[(y * image->width + x) * 4 + 0]);
		}
	}

	return texture;
}
