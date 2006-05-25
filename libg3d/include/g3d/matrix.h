/* $Id: matrix.h,v 1.1.2.3 2006/01/23 16:43:18 dahms Exp $ */

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

#ifndef __G3D_MATRIX_H__
#define __G3D_MATRIX_H__

#include <glib.h>

G_BEGIN_DECLS

/**
 * g3d_matrix_identity:
 * @matrix: 4x4 matrix (float[16])
 *
 * Sets the given matrix to the identity matrix.
 *
 * Returns: TRUE on success, FALSE else
 */
gboolean g3d_matrix_identity(gfloat *matrix);

/**
 * g3d_matrix_multiply:
 * @m1: first matrix
 * @m2: second matrix
 * @rm: resulting matrix
 *
 * Multiplies the matrixes.
 *
 * Returns: TRUE on success, FALSE else
 */
gboolean g3d_matrix_multiply(gfloat *m1, gfloat *m2, gfloat *rm);

/**
 * g3d_matrix_translate:
 * @x: x translation
 * @y: y translation
 * @z: z translation
 * @rm: resulting matrix
 *
 * Adds a translation to the the matrix.
 *
 * Returns: TRUE on success, FALSE else
 */
gboolean g3d_matrix_translate(gfloat x, gfloat y, gfloat z, gfloat *rm);

/**
 * g3d_matrix_rotate:
 * @angle: rotation angle
 * @ax: x component of rotation axis
 * @ay: y component of rotation axis
 * @az: z component of rotation axis
 * @rm: resulting matrix
 *
 * Adds a rotation to the matrix.
 *
 * Returns: TRUE on success, FALSE else
 */
gboolean g3d_matrix_rotate(gfloat angle, gfloat ax, gfloat ay, gfloat az,
	gfloat *rm);

/**
 * g3d_matrix_rotate_xyz
 * @rx: rotation around x axis
 * @ry: rotation around y axis
 * @rz: rotation around z axis
 * @rm: resulting matrix
 *
 * Adds a rotation around the 3 coordinate system axes to the matrix.
 *
 * Returns: TRUE on success, FALSE else
 */
gboolean g3d_matrix_rotate_xyz(gfloat rx, gfloat ry, gfloat rz, gfloat *rm);

G_END_DECLS

#endif /* __G3D_MATRIX_H__ */
