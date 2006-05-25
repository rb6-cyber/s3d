/* $Id: img_sgi.c,v 1.1.2.2 2006/01/23 16:44:28 dahms Exp $ */

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
#include <errno.h>

#include <g3d/types.h>
#include <g3d/read.h>

#define SGI_STORAGE_VERBATIM  0
#define SGI_STORAGE_RLE       1

gboolean plugin_load_image(G3DContext *context, const gchar *filename,
	G3DImage *image, gpointer user_data)
{
	FILE *f;
	guint32 cmap, planes, x, y, p;
	guint16 dims;
	guint8 storage, bpc;
	gchar name[80];

	f = fopen(filename, "rb");
	if(f == NULL)
	{
		g_warning("failed to open file '%s'", filename);
		return FALSE;
	}

	if(g3d_read_int16_be(f) != 474)
	{
		g_warning("file '%s' is not a SGI RGB file", filename);
		fclose(f);
		return FALSE;
	}

	storage = g3d_read_int8(f);
	bpc = g3d_read_int8(f);
	dims = g3d_read_int16_be(f);

	if(bpc != 1)
	{
		g_warning("SGI: %s: bpc != 1 -- unsupported", filename);
		fclose(f);
		return FALSE;
	}

	image->width = g3d_read_int16_be(f);
	image->height = g3d_read_int16_be(f);

	planes = g3d_read_int16_be(f); /* ZSIZE */
	image->depth = 32;
	g3d_read_int32_be(f); /* PIXMIN */
	g3d_read_int32_be(f); /* PIXMAX */
	g3d_read_int32_be(f); /* DUMMY */
	fread(name, 1, 80, f);

	if(strlen(name) > 0)
	{
#if DEBUG > 0
		g_print("SGI: image name: %s\n", name);
#endif
		image->name = g_strdup(name);
	}
	else
	{
		image->name = g_strdup(filename);
	}

	cmap = g3d_read_int32_be(f); /* COLORMAP */
	fseek(f, 404, SEEK_CUR);

#if DEBUG > 0
	g_print("SGI: %dx%dx%d, %d bpc, colormap: 0x%02x\n",
		image->width, image->height, planes, bpc, cmap);
#endif
	/* end of header */

	image->pixeldata = g_new0(guint8, image->width * image->height * 4);

	if(storage == SGI_STORAGE_VERBATIM)
	{
		for(p = 0; p < planes; p ++)
		{
			for(y = 0; y < image->height; y ++)
			{
				for(x = 0; x < image->width; x ++)
				{
					image->pixeldata[(y * image->width + x) * 4 + p] =
						g3d_read_int8(f);

					if(planes == 1)
					{
						/* greyscale: g = r; b = r; */
						image->pixeldata[(y * image->width + x) * 4 + 1] =
							image->pixeldata[(y * image->width + x) * 4];
						image->pixeldata[(y * image->width + x) * 4 + 2] =
							image->pixeldata[(y * image->width + x) * 4];
					}
				} /* x */
			} /* y */
		} /* p */
	} /* verbatim */
	else /* RLE */
	{
		guint32 *starttab, *lengthtab, rleoff, rlelen;
		guint8 cnt, pixel;

		starttab = g_new0(guint32, image->height * planes);
		lengthtab = g_new0(guint32, image->height * planes);

		/* read starttab */
		for(p = 0; p < planes; p ++)
			for(y = 0; y < image->height; y ++)
				starttab[y * planes + p] = g3d_read_int32_be(f);
		/* read lengthtab */
		for(p = 0; p < planes; p ++)
			for(y = 0; y < image->height; y ++)
				lengthtab[y * planes + p] = g3d_read_int32_be(f);

		/* read image data */
		for(p = 0; p < planes; p ++)
			for(y = 0; y < image->height; y ++)
			{
				rleoff = starttab[y * planes + p];
				rlelen = lengthtab[y * planes + p];

				fseek(f, rleoff, SEEK_SET);

				x = 0;

				while(1)
				{
					pixel = g3d_read_int8(f);
					cnt = pixel & 0x7F;

					if(cnt == 0)
						break;

					if(pixel & 0x80)
					{
						/* copy n bytes */
						while(cnt --)
						{
							image->pixeldata[(y * image->width + x) * 4 + p] =
		                        g3d_read_int8(f);
							x ++;
						}
					}
					else
					{
						/* repeat next byte n times */
						pixel = g3d_read_int8(f);
						while(cnt --)
						{
							image->pixeldata[(y * image->width + x) * 4 + p] =
								pixel;
							x ++;
						}
					}
				}
			}

		g_free(starttab);
		g_free(lengthtab);
	}

	/* set alpha to 1.0 */
	if(planes < 4)
		for(y = 0; y < image->height; y ++)
			for(x = 0; x < image->width; x ++)
				image->pixeldata[(y * image->width + x) * 4 + 3] = 0xFF;

	fclose(f);

	return TRUE;
}

gchar *plugin_description(G3DContext *context)
{
	return g_strdup(
		"Plugin to read SGI RGB (.rgb) images\n");
}

gchar **plugin_extensions(G3DContext *context)
{
	return g_strsplit("rgb:sgi", ":", 0);
}

