/* $Id: plugins.c,v 1.1.2.7 2006/01/23 16:38:47 dahms Exp $ */

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

#include <string.h>
#include <unistd.h>

#include <g3d/config.h>
#include <g3d/types.h>
#include <g3d/plugins.h>

static void plugins_free_plugin(G3DPlugin *plugin)
{
	if(plugin->name)
		g_free(plugin->name);
	if(plugin->path)
		g_free(plugin->path);
	if(plugin->extensions)
		g_strfreev(plugin->extensions);

	if(plugin->module)
		g_module_close(plugin->module);

	g_free(plugin);
}

#define PLUGIN_GET_SYMBOL(symbol, pointer) \
	if(g_module_symbol(plugin->module, symbol, \
		(gpointer *)&(pointer)) != TRUE) \
		pointer = NULL; \
	/* else \
		g_print("%s: symbol %s found\n", plugin->name, symbol); */

static gboolean plugins_loaddirectory(G3DContext *context,
	const gchar *dirname)
{
	GDir *plugindir;
	G3DPlugin *plugin;
	gchar **last, **dir, **dirnames;
	gchar **ext, **exts;
	gchar *path;
	const gchar *filename;
	guint32 type = G3D_PLUGIN_UNKNOWN;

	plugindir = g_dir_open(dirname, 0, NULL);
	if(!plugindir)
		return FALSE;

	dirnames = g_strsplit(dirname, "/", 0);
	dir = last = dirnames;

	while(*dir != NULL)
	{
		if(*dir != NULL)
			last = dir;

		dir ++;
	}

	if(strcmp("import", *last) == 0)
		type = G3D_PLUGIN_IMPORT;
	else if(strcmp("image", *last) == 0)
		type = G3D_PLUGIN_IMAGE;

	g_strfreev(dirnames);

	while((filename = g_dir_read_name(plugindir)) != NULL)
	{
		if(g_strcasecmp(filename + strlen(filename) - 3, ".la") == 0)
		{
			plugin = g_new0(G3DPlugin, 1);

			plugin->name = g_strdup(filename);
			plugin->path = g_strdup(dirname);
			plugin->type = type;

			path = g_strdup_printf("%s/%s", dirname, filename);

			plugin->module = g_module_open(path, 0);
			if(plugin->module == NULL)
			{
				g_warning("libg3d: plugins: failed to load %s: %s\n",
					path, g_module_error());

				plugins_free_plugin(plugin);
			}
			else
			{
				PLUGIN_GET_SYMBOL("plugin_extensions", plugin->ext_func);
				PLUGIN_GET_SYMBOL("plugin_description", plugin->desc_func);
				PLUGIN_GET_SYMBOL("plugin_init", plugin->init_func);
				PLUGIN_GET_SYMBOL("plugin_cleanup", plugin->cleanup_func);
				PLUGIN_GET_SYMBOL("plugin_load_model", plugin->loadmodel_func);
				PLUGIN_GET_SYMBOL("plugin_load_image", plugin->loadimage_func);

				/* append plugin to list */
				context->plugins = g_slist_append(context->plugins, plugin);

				/* handle managed extensions */
				if(plugin->ext_func)
				{
					ext = exts = plugin->ext_func(context);
					while(*ext != NULL)
					{
						if(plugin->type == G3D_PLUGIN_IMAGE)
							g_hash_table_insert(context->exts_image,
								g_strdup(*ext), plugin);
						else if(plugin->type == G3D_PLUGIN_IMPORT)
							g_hash_table_insert(context->exts_import,
								g_strdup(*ext), plugin);

						ext ++;
					}

					plugin->extensions = exts;
				}

				/* init plugin if possible */
				if(plugin->init_func)
				{
					plugin->user_data = plugin->init_func(context);
				}

			} /* plugin->module != NULL */

			g_free(path);
		} /* .la file */

	} /* while g_dir_read_name(...) */

	g_dir_close(plugindir);

	return TRUE;
}

#undef PLUGIN_GET_SYMBOL

gboolean g3d_plugins_init(G3DContext *context)
{
	context->exts_import = g_hash_table_new(g_str_hash, g_str_equal);
	context->exts_image = g_hash_table_new(g_str_hash, g_str_equal);

	plugins_loaddirectory(context, PLUGIN_DIR "/image");
	plugins_loaddirectory(context, PLUGIN_DIR "/import");

	return TRUE;
}

void g3d_plugins_cleanup(G3DContext *context)
{
	/* TODO: implement */
}

gchar *g3d_plugins_get_filetype(const gchar *filename)
{
	gchar *ext, *lcext;

	ext = strrchr(filename, '.');
	if(ext == NULL)
	{
		g_warning("can't determine file type: %s", filename);
		return NULL;
	}
	ext ++;
	if(strlen(ext) == 0)
	{
		g_warning("zero length file extension: %s", filename);
		return NULL;
	}

	lcext = g_strdup(ext);
	g_strdown(lcext);

	return lcext;
}

gboolean g3d_plugins_load_model(G3DContext *context, const gchar *filename,
	G3DModel *model)
{
	G3DPlugin *plugin;
	gchar *lcext, *basename, *dirname;
	gboolean retval;

	lcext = g3d_plugins_get_filetype(filename);
	if(lcext == NULL)
		return FALSE;

	plugin = g_hash_table_lookup(context->exts_import, lcext);
	if(plugin == NULL)
	{
		g_warning("no handler for filetype '.%s' found", lcext);
		g_free(lcext);
		return FALSE;
	}
	g_free(lcext);

	if(plugin->loadmodel_func == NULL)
	{
		g_warning("can't find symbol 'plugin_load_model' in %s", plugin->name);
		return FALSE;
	}

	basename = g_path_get_basename(filename);
	dirname = g_path_get_dirname(filename);

	chdir(dirname);

	retval = plugin->loadmodel_func(context, basename, model,
		plugin->user_data);

	g_free(basename);
	g_free(dirname);

	return retval;
}

gboolean g3d_plugins_load_image(G3DContext *context, const gchar *filename,
	G3DImage *image)
{
	G3DPlugin *plugin;
	gchar *lcext;
	gboolean retval;

	lcext = g3d_plugins_get_filetype(filename);
    if(lcext == NULL)
		return FALSE;

	plugin = g_hash_table_lookup(context->exts_image, lcext);
	if(plugin == NULL)
	{
		g_warning("no handler for filetype '.%s' found", lcext);
		g_free(lcext);
		return FALSE;
	}
	g_free(lcext);

	if(plugin->loadimage_func == NULL)
	{
		g_warning("can't find symbol 'plugin_load_image' in %s", plugin->name);
		return FALSE;
	}

	retval = plugin->loadimage_func(context, filename, image,
		plugin->user_data);

	return retval;
}

struct plugins_ext_list {
	guint32 pos;
	gchar **list;
};

static void plugins_list_ext(gpointer key, gpointer value, gpointer user_data)
{
	struct plugins_ext_list *data;

	data = (struct plugins_ext_list *)user_data;

	data->list[data->pos] = g_strdup(key);
	data->pos ++;
}

gchar **g3d_plugins_get_image_extensions(G3DContext *context)
{
	gchar **list;
	guint32 n;
	struct plugins_ext_list data;

	n = g_hash_table_size(context->exts_image);
	list = g_new0(gchar *, n + 1);

	data.pos = 0;
	data.list = list;

	/* fill extension list */
	g_hash_table_foreach(context->exts_image, plugins_list_ext, &data);

	return list;
}

