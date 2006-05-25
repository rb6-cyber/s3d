#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <g3d/types.h>
#include <g3d/read.h>

gboolean plugin_load_image(G3DContext *context, const gchar *filename,
	G3DImage *image, gpointer user_data)
{
	FILE *f;
	guint32 filesize, offset, headsize, compression;
	guint32 x, y;

	f = fopen(filename, "r");
	if(f == NULL)
	{
		g_printerr("couldn't open image file '%s': %s\n", filename,
			strerror(errno));
		return FALSE;
	}

	/* bitmap file always starts with 'BM' */
	if(g3d_read_int16_le(f) != ('B' | ('M' << 8)))
	{
		g_printerr("bitmap magic not found: image seems to be corrupt\n");
		return FALSE;
	}

	image->name = g_strdup(filename);

	filesize = g3d_read_int32_le(f);      /* file size */
	g3d_read_int32_le(f);                 /* 2 x UINT16 reserved */
	offset   = g3d_read_int32_le(f);      /* offset of data */
	headsize = g3d_read_int32_le(f);      /* size of header */
	image->width  = g3d_read_int32_le(f); /* width */
	image->height = g3d_read_int32_le(f); /* height */
	g3d_read_int16_le(f);                               /* num of color planes */
	image->depth  = g3d_read_int16_le(f); /* bits per pixel */
	compression   = g3d_read_int32_le(f); /* compression */
	g3d_read_int32_le(f);                 /* image size */
	g3d_read_int32_le(f);                 /* v/res (dpi) */
	g3d_read_int32_le(f);                 /* h/res (dpi) */

	fseek(f, offset, SEEK_SET);

#define ALL32BIT
#ifndef ALL32BIT /* always 32bit for now.. */
	image->pixeldata = g_malloc0(image->width*image->height*(image->depth/8));
#else
	image->pixeldata = g_malloc0(image->width * image->height * 4);
#endif

	for(y=image->height-1; y>=0; y--)
	{
#if DEBUG > 5
		g_printerr("| ");
#endif
		for(x=0; x<image->width; x++)
		{
			switch(image->depth)
			{
				case 8:
#ifndef ALL32BIT
					image->pixeldata[y*image->width+x] = g3d_read_int8(f);
#else
					image->pixeldata[(y*image->width+x)*4+0] = g3d_read_int8(f);
					image->pixeldata[(y*image->width+x)*4+3] = 0xFF;
#endif
#if DEBUG > 5
					g_printerr("%2.2X ", image->pixeldata[y*image->width+x]);
#endif
					break;
				case 24:
					image->pixeldata[(y*image->width+x)*4+0] = g3d_read_int8(f);
					image->pixeldata[(y*image->width+x)*4+1] = g3d_read_int8(f);
					image->pixeldata[(y*image->width+x)*4+2] = g3d_read_int8(f);
					image->pixeldata[(y*image->width+x)*4+3] = 0xFF;
					break;
				default:
					break;
			}
		}
#if DEBUG > 5
		g_printerr("\n");
#endif
	}
	image->depth = 32;
#if DEBUG > 2
	g_printerr("bitmap successfully loaded\n");
#endif

	return TRUE;
}

gchar *plugin_description(G3DContext *context)
{ 
	return g_strdup(
		"Plugin to read Windows Bitmap (.bmp) images\n"
		"(for use as textures or height fields)\n"
		"Author: Markus Dahms");
}

gchar **plugin_extensions(G3DContext *context)
{
	return g_strsplit("bmp", ":", 0);
}

