/* $Id: types.h,v 1.1.2.4 2006/01/23 16:43:19 dahms Exp $ */

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

#ifndef __G3D_TYPES_H__
#define __G3D_TYPES_H__

#include <glib.h>

G_BEGIN_DECLS

/*****************************************************************************
 * G3DImage
 *****************************************************************************/

#define G3D_FLAG_IMG_GREYSCALE  (1L << 1)

typedef struct {
	gchar *name;
	guint32 width;
	guint32 height;
	guint8 depth;
	guint32 flags;
	guint8 *pixeldata;

	guint32 tex_id;
	gfloat tex_scale_u;
	gfloat tex_scale_v;
} G3DImage;

/*****************************************************************************
 * G3DMaterial
 *****************************************************************************/

#define G3D_FLAG_MAT_TWOSIDE    (1L << 0)

typedef struct {
	gchar *name;
	gfloat r, g, b, a;
	gfloat shininess;
	gfloat specular[4];
	guint32 flags;

	G3DImage *tex_image;
} G3DMaterial;

/*****************************************************************************
 * G3DFace
 *****************************************************************************/

#define G3D_FLAG_FAC_NORMALS    (1L << 0)
#define G3D_FLAG_FAC_TEXMAP     (1L << 1)

typedef struct {
	guint32 vertex_count;
	guint32 *vertex_indices;

	G3DMaterial *material;

	guint32 flags;

	gfloat *normals;

	G3DImage *tex_image;
	guint32 tex_vertex_count;
	gfloat *tex_vertex_data;
} G3DFace;

/*****************************************************************************
 * G3DObject
 *****************************************************************************/
typedef struct {
	gchar *name;

	GSList *materials;
	GSList *faces;
	GSList *objects;

	/* don't render this object */
	gboolean hide;

	/* vertices */
	guint32 vertex_count;
	gfloat *vertex_data;

	/* texture stuff: temporary storage, should be in G3DFace items */
	guint32 tex_vertex_count;
	gfloat *tex_vertex_data;
	G3DImage *tex_image;

	/* some fields to speed up rendering, should not be used by plugins */
	/* FIXME: remove from API (replace with user_data pointer?) */
	gfloat *_normals;
	G3DMaterial **_materials;
	guint32  _num_faces;
	guint32 *_indices;
	guint32 *_flags;
	guint32 *_tex_images;
	gfloat *_tex_coords;
} G3DObject;

/*****************************************************************************
 * G3DContext
 *****************************************************************************/

typedef gboolean (* G3DSetBgColorFunc)(gfloat r, gfloat g, gfloat b, gfloat a,
	gpointer user_data);

typedef gboolean (* G3DUpdateInterfaceFunc)(gpointer user_data);

typedef gboolean (* G3DUpdateProgressBarFunc)(gfloat percentage,
	gboolean show, gpointer user_data);

typedef struct {
	GSList *plugins;

	GHashTable *exts_import;
	GHashTable *exts_image;

	G3DSetBgColorFunc set_bgcolor_func;
	gpointer set_bgcolor_data;
	G3DUpdateInterfaceFunc update_interface_func;
	gpointer update_interface_data;
	G3DUpdateProgressBarFunc update_progress_bar_func;
	gpointer update_progress_bar_data;
} G3DContext;

/*****************************************************************************
 * G3DModel
 *****************************************************************************/

typedef struct {
	gchar *filename;
	G3DContext *context;

	GSList *materials;
	GSList *objects;

	GHashTable *tex_images;
} G3DModel;

G_END_DECLS

#endif /* __G3D_TYPES_H__ */
