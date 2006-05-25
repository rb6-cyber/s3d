/* $Id: object.h,v 1.1.2.2 2006/01/23 16:43:19 dahms Exp $ */

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

#ifndef __G3D_OBJECT_H__
#define __G3D_OBJECT_H__

#include <g3d/types.h>

G_BEGIN_DECLS

/**
 * g3d_object_free:
 * @object: the object to free
 *
 * Frees all memory allocated for that object.
 */
void g3d_object_free(G3DObject *object);

/**
 * g3d_object_radius:
 * @object: the object to measure
 *
 * Calculates the radius of the object. This is the maximum from the
 * center to a vertex.
 * 
 * Returns: the radius of the given object
 */
gdouble g3d_object_radius(G3DObject *object);

/**
 * g3d_object_scale:
 * @object: the object to scale
 * @scale: scale factor
 *
 * Resizes the object by the factor @scale.
 * 
 * Returns: TRUE on success, FALSE else
 */
gboolean g3d_object_scale(G3DObject *object, gfloat scale);

/**
 * g3d_object_transform:
 * @object: the object to transform
 * @matrix: the transformation matrix
 *
 * Multiplies all vertices of the object with the transformation matrix.
 *
 * Returns: TRUE on success, FALSE else
 */
gboolean g3d_object_transform(G3DObject *object, gfloat *matrix);

/**
 * g3d_object_duplicate:
 * @object: the object to duplicate
 *
 * Duplicates an object with all vertices, faces and materials.
 *
 * Returns: the new clone object
 */
G3DObject *g3d_object_duplicate(G3DObject *object);

/**
 * g3d_object_optimize:
 * @object: the object to optimize
 *
 * Puts all vertex and face information into special arrays for faster
 * rendering.
 *
 * Returns: TRUE on success, FALSE else
 */
gboolean g3d_object_optimize(G3DObject *object);

/**
 * g3d_object_smooth:
 * @object: the object to smooth
 *
 * FIXME: unimplemented.
 *
 * Returns: TRUE on success, FALSE else
 */
gboolean g3d_object_smooth(G3DObject *object);

/**
 * g3d_object_merge:
 * @o1: first and target object
 * @o2: second object
 *
 * Merges both objects into @o1.
 * FIXME: needs cleanup
 *
 * Returns: TRUE on success, FALSE else
 */
gboolean g3d_object_merge(G3DObject *o1, G3DObject *o2);

G_END_DECLS

#endif /* __G3D_OBJECT_H__ */
