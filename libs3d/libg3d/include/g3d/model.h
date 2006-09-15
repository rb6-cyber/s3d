/* $Id: model.h,v 1.1.2.4 2006/01/23 16:43:18 dahms Exp $ */

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

#ifndef __G3D_MODEL_H__
#define __G3D_MODEL_H__

#include <g3d/types.h>

G_BEGIN_DECLS

/**
 * g3d_model_new:
 *
 * This functions allocates and initializes a new G3DModel.
 *
 * Returns: a newly allocated G3DModel
 */
G3DModel *g3d_model_new(void);

/**
 * g3d_model_load:
 * @context: a valid context
 * @filename: the file name of the model to load
 *
 * Loads a model from a file. The model is checked, centered, resized,
 * optimized.
 *
 * Returns: the loaded model or NULL in case of an error
 */
G3DModel *g3d_model_load(G3DContext *context, const gchar *filename);

/**
 * g3d_model_check:
 * @model: the model to check
 *
 * Checks whether a model returned by plugin is valid.
 *
 * Returns: TRUE on success, FALSE on error
 */
gboolean g3d_model_check(G3DModel *model);

/**
 * g3d_model_center:
 * @model: the model to center
 *
 * Translates all object coordinates that the object center is at (0, 0, 0)
 *
 * Returns: TRUE on success, FALSE on error
 */
gboolean g3d_model_center(G3DModel *model);

/**
 * g3d_model_clear:
 * @model: the model to clear
 *
 * Removes all objects from a model.
 */
void g3d_model_clear(G3DModel *model);

/**
 * g3d_model_free:
 * @model: the model to free
 *
 * Frees all memory allocated for the model including all objects, materials
 * and textures.
 */
void g3d_model_free(G3DModel *model);

/**
 * g3d_model_get_object_by_name:
 * @model: the model containing all objects
 * @name: the name of the requested object
 *
 * Searches the object tree for an object with the given name.
 *
 * Returns: the requested object or NULL if non was found
 */
G3DObject *g3d_model_get_object_by_name(G3DModel *model, const gchar *name);

G_END_DECLS

#endif /* __G3D_MODEL_H__ */
