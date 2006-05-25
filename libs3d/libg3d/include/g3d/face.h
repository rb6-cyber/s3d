/* $Id: face.h,v 1.1.2.4 2006/01/23 16:43:18 dahms Exp $ */

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

#ifndef __G3D_FACE_H__
#define __G3D_FACE_H__

#include <g3d/types.h>

G_BEGIN_DECLS

/**
 * g3d_face_free:
 * @face: the face to free
 *
 * Frees all memory allocated for this face.
 */
void g3d_face_free(G3DFace *face);

/**
 * g3d_face_get_normal:
 * @face: face to calculate normal of
 * @object: object containing vertices of face
 * @nx: x component of resulting normal
 * @ny: y component of resulting normal
 * @nz: z component of resulting normal
 *
 * calculates the normal of a face.
 *
 * Returns: TRUE on success, FALSE else
 */
gboolean g3d_face_get_normal(G3DFace *face, G3DObject *object,
	gfloat *nx, gfloat *ny, gfloat *nz);

G_END_DECLS

#endif /* __G3D_FACE_H__ */
