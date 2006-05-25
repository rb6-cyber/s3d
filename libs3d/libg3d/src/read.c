/* $Id: read.c,v 1.1.2.3 2006/01/23 16:38:47 dahms Exp $ */

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
#include <glib.h>

gint32 g3d_read_int8(FILE *f)
{
	int c = fgetc(f);
	if(c == EOF)
		return 0;
	else
		return c;
}

gint32 g3d_read_int16_be(FILE *f)
{
	return (g3d_read_int8(f)<<8) | g3d_read_int8(f);
}

gint32 g3d_read_int16_le(FILE *f)
{
	return g3d_read_int8(f) | (g3d_read_int8(f)<<8);
}

gint32 g3d_read_int32_be(FILE *f)
{
	return (g3d_read_int8(f) << 24) | (g3d_read_int8(f) << 16) |
		(g3d_read_int8(f) << 8) | g3d_read_int8(f);
}

gint32 g3d_read_int32_le(FILE *f)
{
	return  g3d_read_int8(f) | (g3d_read_int8(f) << 8) |
		(g3d_read_int8(f) << 16) | (g3d_read_int8(f) << 24);
}

gfloat g3d_read_float_be(FILE *f)
{
	union {
		gfloat f;
		guint8 u[4];
	} u;

	u.u[3] = g3d_read_int8(f);
	u.u[2] = g3d_read_int8(f);
	u.u[1] = g3d_read_int8(f);
	u.u[0] = g3d_read_int8(f);

	return u.f;
}

gfloat g3d_read_float_le(FILE *f)
{
	union {
		gfloat f;
		guint8 u[4];
	} u;

	u.u[0] = g3d_read_int8(f);
	u.u[1] = g3d_read_int8(f);
	u.u[2] = g3d_read_int8(f);
	u.u[3] = g3d_read_int8(f);

	return u.f;
}

