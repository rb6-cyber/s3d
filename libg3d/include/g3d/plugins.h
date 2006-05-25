/* $Id: plugins.h,v 1.1.2.3 2006/01/23 16:43:19 dahms Exp $ */

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

#ifndef __G3D_PLUGINS_H__
#define __G3D_PLUGINS_H__

#include <gmodule.h>
#include <g3d/types.h>

G_BEGIN_DECLS

#define G3D_PLUGIN_UNKNOWN   0x00
#define G3D_PLUGIN_IMPORT    0x01
#define G3D_PLUGIN_IMAGE     0x02

typedef gpointer (* PluginInitFunc)(G3DContext *context);

typedef void (* PluginCleanupFunc)(gpointer user_data);

typedef gboolean (* PluginLoadModelFunc)(G3DContext *context,
	const gchar *filename, G3DModel *model, gpointer user_data);

typedef gboolean (* PluginLoadImageFunc)(G3DContext *context,
	const gchar *filename, G3DImage *image, gpointer user_data);

typedef gchar *(* PluginGetDescFunc)(G3DContext *context);

typedef gchar **(* PluginGetExtFunc)(G3DContext *context);

typedef struct {
	gchar *name;
	gchar *path;
	guint32 type;
	gchar **extensions;

	PluginInitFunc init_func;
	PluginCleanupFunc cleanup_func;
	PluginLoadModelFunc loadmodel_func;
	PluginLoadImageFunc loadimage_func;
	PluginGetDescFunc desc_func;
	PluginGetExtFunc ext_func;

	gpointer user_data;

	GModule *module;
} G3DPlugin;

gboolean g3d_plugins_init(G3DContext *context);

void g3d_plugins_cleanup(G3DContext *context);

gboolean g3d_plugins_load_model(G3DContext *context, const gchar *filename,
	G3DModel *model);

gboolean g3d_plugins_load_image(G3DContext *context, const gchar *filename,
	G3DImage *image);

gchar **g3d_plugins_get_image_extensions(G3DContext *context);

G_END_DECLS

#endif /* __G3D_PLUGINS_H__ */
