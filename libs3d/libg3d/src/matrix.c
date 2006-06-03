/* $Id: matrix.c,v 1.1.2.2 2006/01/23 16:38:47 dahms Exp $ */

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

#include <string.h>
#include <math.h>
#include <glib.h>
#include <g3d/vector.h>

gboolean g3d_matrix_identity(gfloat *matrix)
{
	static gfloat identity[16] = {
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0 };

	memcpy(matrix, identity, sizeof(gfloat) * 16);
	return TRUE;
}

gboolean g3d_matrix_multiply(gfloat *m1, gfloat *m2, gfloat *rm)
{
	guint32 i, j;
	gfloat matrix[16];

	for(i = 0; i < 4; i ++)
		for(j = 0; j < 4; j ++)
			matrix[i * 4 + j] =
				m2[0 * 4 + j] * m1[i * 4 + 0] +
				m2[1 * 4 + j] * m1[i * 4 + 1] +
				m2[2 * 4 + j] * m1[i * 4 + 2] +
				m2[3 * 4 + j] * m1[i * 4 + 3];

	memcpy(rm, matrix, 16 * sizeof(gfloat));
	return TRUE;
}

gboolean g3d_matrix_rotate(gfloat angle, gfloat ax, gfloat ay, gfloat az,
	gfloat *rm)
{
	g3d_vector_unify(&ax, &ay, &az);
	g3d_matrix_identity(rm);

	rm[0 * 4 + 0] = cos(angle) + (ax * ax) * (1 - cos(angle));
	rm[0 * 4 + 1] = ax * ay * (1 - cos(angle)) - az * sin(angle);
	rm[0 * 4 + 2] = ax * az * (1 - cos(angle)) + ay * sin(angle);

	rm[1 * 4 + 0] = ay * ax * (1 - cos(angle)) + az * sin(angle);
	rm[1 * 4 + 1] = cos(angle) + (ay * ay) * (1 - cos(angle));
	rm[1 * 4 + 2] = ay * az * (1 - cos(angle)) - ax * sin(angle);

	rm[2 * 4 + 0] = az * ax * (1 - cos(angle)) - ay * sin(angle);
	rm[2 * 4 + 1] = az * ay * (1 - cos(angle)) + ax * sin(angle);
	rm[2 * 4 + 2] = cos(angle) + (az * az) * (1 - cos(angle));

	return TRUE;
}

gboolean g3d_matrix_rotate_xyz(gfloat rx, gfloat ry, gfloat rz, gfloat *rm)
{
	gfloat matrix[16];

	g3d_matrix_identity(rm);

	g3d_matrix_rotate(rx, 1.0, 0.0, 0.0, matrix);
	g3d_matrix_multiply(matrix, rm, rm);

	g3d_matrix_rotate(ry, 0.0, 1.0, 0.0, matrix);
	g3d_matrix_multiply(matrix, rm, rm);

	g3d_matrix_rotate(rz, 0.0, 0.0, 1.0, matrix);
	g3d_matrix_multiply(matrix, rm, rm);

	return TRUE;
}

gboolean g3d_matrix_translate(gfloat x, gfloat y, gfloat z, gfloat *rm)
{
	guint32 i;

	for(i = 0; i < 4; i ++)
		rm[3 * 4 + i] =
			rm[0 * 4 + i] * x +
			rm[1 * 4 + i] * y +
			rm[2 * 4 + i] * z +
			rm[3 * 4 + i];

	return TRUE;
}