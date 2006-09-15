/* $Id: primitive.h,v 1.1.2.3 2006/01/23 16:43:19 dahms Exp $ */

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

#ifndef __G3D_PRIMITIVE_H__
#define __G3D_PRIMITIVE_H__

#include <g3d/types.h>

G_BEGIN_DECLS

G3DObject *g3d_primitive_cube(gfloat width, gfloat height, gfloat depth,
	G3DMaterial *material);

/**
 * g3d_primitive_cylinder:
 * @radius: the radius of the cylinder
 * @height: the height of the side faces
 * @sides: number of side faces (number of circle segments)
 * @top: add top faces
 * @bottom: add bottom faces
 * @material: material to use for faces
 *
 * Generates an object containing a cylinder.
 *
 * Returns: cylinder object
 */
G3DObject *g3d_primitive_cylinder(gfloat radius, gfloat height,
	guint32 sides, gboolean top, gboolean bottom, G3DMaterial *material);

/**
 * g3d_primitive_tube:
 * @r_in: inner radius
 * @r_out: outer radius
 * @height: the height of the side faces
 * @sides: number of side faces (number of circle segments)
 * @top: add top faces
 * @bottom: add bottom faces
 * @material: material to use for faces
 *
 * Generates an object containing a tube (a cylinder with a hole).
 *
 * Returns: tube object
 */
G3DObject *g3d_primitive_tube(gfloat r_in, gfloat r_out, gfloat height,
	guint32 sides, gboolean top, gboolean bottom, G3DMaterial *material);

/**
 * g3d_primitive_sphere:
 * @radius: radius
 * @vseg: number of vertical segments
 * @hseg: number of horizontal segments
 * @materal: material to use for faces
 *
 * Generates an object containing a sphere.
 *
 * Returns: sphere object
 */
G3DObject *g3d_primitive_sphere(gfloat radius, guint32 vseg, guint32 hseg,
	G3DMaterial *material);

G_END_DECLS

#endif /* __G3D_PRIMITIVE_H__ */
