/* $Id: iff.c,v 1.1.2.2 2006/01/23 16:38:46 dahms Exp $ */

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
#include <g3d/read.h>
#include <g3d/iff.h>

FILE *g3d_iff_open(const gchar *filename, guint32 *id, guint32 *len)
{
	FILE *f;
	guint32 form_bytes;

	f = fopen(filename, "r");
	if(f == NULL)
	{
		g_warning("can't open file '%s'", filename);
		return NULL;
	}

	if(g3d_read_int32_be(f) != G3D_IFF_MKID('F','O','R','M'))
	{
		g_warning("file %s is not an IFF file", filename);
		fclose(f);
		return NULL;
	}

	form_bytes = g3d_read_int32_be(f);
	*id = g3d_read_int32_be(f);
	form_bytes -= 4;
	*len = form_bytes;

	return f;
}

int g3d_iff_readchunk(FILE *f, guint32 *id, guint32 *len)
{
	*id = g3d_read_int32_be(f);
	*len = g3d_read_int32_be(f);
	return 8 + *len + (*len % 2);
}

