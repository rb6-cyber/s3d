/* $Id: vector.c,v 1.1.2.2 2006/01/23 16:38:47 dahms Exp $ */

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

#include <math.h>
#include <glib.h>

gboolean g3d_vector_normal(gfloat ax, gfloat ay, gfloat az,
	gfloat bx, gfloat by, gfloat bz,
	gfloat *nx, gfloat *ny, gfloat *nz)
{
	*nx = ay * bz - az * by;
	*ny = az * bx - ax * bz;
	*nz = ax * by - ay * bx;

	return TRUE;
}

gboolean g3d_vector_unify(gfloat *nx, gfloat *ny, gfloat *nz)
{
	gfloat r;

	r = sqrt(*nx * *nx + *ny * *ny + *nz * *nz);
	if(r < 0.000001) return FALSE;

	*nx /= r;
	*ny /= r;
	*nz /= r;

	return TRUE;
}

gboolean g3d_vector_transform(gfloat *x, gfloat *y, gfloat *z, gfloat *matrix)
{
	gfloat vector[4], result[4];
	guint32 i, k;

	vector[0] = *x;
	vector[1] = *y;
	vector[2] = *z;
	vector[3] = 1.0;

	for(i = 0; i < 4; i ++)
	{
		result[i] = 0.0;

		for(k = 0; k < 4; k ++)
			result[i] += matrix[i * 4 + k] * vector[k];
	}

	*x = result[0];
	*y = result[1];
	*z = result[2];

	return TRUE;
}

