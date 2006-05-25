/* $Id: img_gdkpixbuf.c,v 1.1.2.3 2006/01/23 16:44:28 dahms Exp $ */

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

#include <gdk-pixbuf/gdk-pixbuf.h>

#include <g3d/types.h>

gboolean plugin_load_image(G3DContext *context, const gchar *filename,
	G3DImage *image, gpointer user_data)
{
	GdkPixbuf *pixbuf;
	guint32 x, y, nchannels;
	guchar *p;

	pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
	if(pixbuf == NULL)
	{
		g_warning("failed to load '%s'", filename);
		return FALSE;
	}

	if(gdk_pixbuf_get_colorspace(pixbuf) != GDK_COLORSPACE_RGB)
	{
		g_warning("GdkPixbuf: %s: colorspace is not RGB", filename);
		gdk_pixbuf_unref(pixbuf);
		return FALSE;
	}

	nchannels = gdk_pixbuf_get_n_channels(pixbuf);
	if(nchannels < 3)
	{
		g_warning("GdkPixbuf: %s: has only %d channels", filename,
			gdk_pixbuf_get_n_channels(pixbuf));
		gdk_pixbuf_unref(pixbuf);
		return FALSE;
	}

	image->width = gdk_pixbuf_get_width(pixbuf);
	image->height = gdk_pixbuf_get_height(pixbuf);
	image->depth = 32;
	image->name = g_path_get_basename(filename);
	image->pixeldata = g_new0(guint8, image->width * image->height * 4);

	for(y = 0; y < image->height; y ++)
		for(x = 0; x < image->width; x ++)
		{
			p = gdk_pixbuf_get_pixels(pixbuf) + y *
				gdk_pixbuf_get_rowstride(pixbuf) + x * nchannels;

			image->pixeldata[(y * image->width + x) * 4 + 0] = p[0];
			image->pixeldata[(y * image->width + x) * 4 + 1] = p[1];
			image->pixeldata[(y * image->width + x) * 4 + 2] = p[2];
			if(gdk_pixbuf_get_n_channels(pixbuf) >= 4)
				image->pixeldata[(y * image->width + x) * 4 + 3] = p[3];
		}

	/* set alpha to 1.0 */
	if(gdk_pixbuf_get_n_channels(pixbuf) < 4)
		for(y = 0; y < image->height; y ++)
			for(x = 0; x < image->width; x ++)
				image->pixeldata[(y * image->width + x) * 4 + 3] = 0xFF;

	gdk_pixbuf_unref(pixbuf);

#if DEBUG > 0
	g_print("GdkPixbuf: image '%s' loaded (%dx%d)\n",
		image->name, image->width, image->height);
#endif

	return TRUE;
}

gchar *plugin_description(G3DContext *context)
{
	return g_strdup("plugin to read images using the GdkPixbuf library");
}

gchar **plugin_extensions(G3DContext *context)
{
	gchar *extensions = g_strdup("");
	gchar **retval;
	gchar *tmp;
	GSList *fitem;
	GdkPixbufFormat *format;

	fitem = gdk_pixbuf_get_formats();
	while(fitem)
	{
		format = (GdkPixbufFormat *)fitem->data;
		tmp = g_strdup_printf("%s%s%s", extensions,
			strlen(extensions) ? ":" : "",
			g_strjoinv(":", gdk_pixbuf_format_get_extensions(format)));
		g_free(extensions);
		extensions = tmp;
		fitem = fitem->next;
	}

	retval = g_strsplit(extensions, ":", 0);
	g_free(extensions);
	return retval;
}
