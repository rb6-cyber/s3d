/* $Id: texture.h,v 1.1.2.4 2006/01/23 16:43:19 dahms Exp $ */

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

#ifndef __G3D_TEXTURE_H__
#define __G3D_TEXTURE_H__

#include <g3d/types.h>

G_BEGIN_DECLS

G3DImage *g3d_texture_load(G3DContext *context, const gchar *filename);

/**
 * g3d_texture_load_cached:
 * @context: a valid context
 * @model: a valid model
 * @filename: the file name of the texture to load
 *
 * Loads a texture image from file and attaches it to a hash table in the
 * model. On a second try to load this texture it is returned from cache.
 *
 * Returns: the texture image
 */
G3DImage *g3d_texture_load_cached(G3DContext *context, G3DModel *model,
	const gchar *filename);

void g3d_texture_free(G3DImage *texture);

/**
 * g3d_texture_prepare:
 * @texture: a texture image
 *
 * Resizes the image to dimensions which are a power of 2 to be
 * usable as an OpenGL texture.
 * (FIXME: unimplemented)
 *
 * Returns: TRUE on success, FALSE else
 */
gboolean g3d_texture_prepare(G3DImage *texture);

gboolean g3d_texture_flip_y(G3DImage *texture);

G3DImage *g3d_texture_merge_alpha(G3DImage *image, G3DImage *aimage);

G_END_DECLS

#endif /* __G3D_TEXTURE_H__ */
