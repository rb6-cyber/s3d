/* $Id: vector.h,v 1.1.2.3 2006/01/23 16:43:19 dahms Exp $ */

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

#ifndef __G3D_VECTOR_H__
#define __G3D_VECTOR_H__

#include <glib.h>

G_BEGIN_DECLS

/**
 * g3d_vector_normal:
 * @ax: x component first vector
 * @ay: y component first vector
 * @az: z component first vector
 * @bx: x component second vector
 * @by: y component second vector
 * @bz: z component second vector
 * @nx: x component resulting normal
 * @ny: y component resulting normal
 * @nz: z component resulting normal
 *
 * calculate the normal from a plane defined by two vectors
 *
 * Returns: TRUE on success, FALSE else
 */
gboolean g3d_vector_normal(gfloat ax, gfloat ay, gfloat az,
	gfloat bx, gfloat by, gfloat bz,
	gfloat *nx, gfloat *ny, gfloat *nz);

/**
 * g3d_vector_unify:
 * @nx: x component of vector
 * @ny: y component of vector
 * @nz: z component of vector
 *
 * Transforms the given vector to the unit vector.
 *
 * Returns: TRUE on success, FALSE else
 */
gboolean g3d_vector_unify(gfloat *nx, gfloat *ny, gfloat *nz);

/**
 * g3d_vector_transform:
 * @x: x component of vector
 * @y: y component of vector
 * @z: z component of vector
 * @matrix: transformation matrix (4x4)
 *
 * Transforms the given vector corresponding to the given matrix
 *
 * Returns: TRUE on success, FALSE else
 */
gboolean g3d_vector_transform(gfloat *x, gfloat *y, gfloat *z, gfloat *matrix);

G_END_DECLS

#endif /* __G3D_VECTOR_H__ */
