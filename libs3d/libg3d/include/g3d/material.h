/* $Id: material.h,v 1.1.2.3 2006/01/23 16:43:18 dahms Exp $ */

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

#ifndef __G3D_MATERIAL_H__
#define __G3D_MATERIAL_H__

#include <g3d/types.h>

G_BEGIN_DECLS

/**
 * g3d_material_new
 *
 * Generates a new material with a default color.
 *
 * Returns: the new material or NULL on error
 */
G3DMaterial *g3d_material_new(void);

/**
 * g3d_material_free:
 * @material: the material to free
 *
 * Frees all memory allocated for that material.
 */
void g3d_material_free(G3DMaterial *material);

G_END_DECLS

#endif /* __G3D_MATERIAL_H__ */
