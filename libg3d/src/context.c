/* $Id: context.c,v 1.1.2.4 2006/01/23 16:38:46 dahms Exp $ */

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

#include <g3d/types.h>
#include <g3d/plugins.h>

G3DContext *g3d_context_new(void)
{
	G3DContext *context;

	context = g_new0(G3DContext, 1);

	g3d_plugins_init(context);

	return context;
}

void g3d_context_free(G3DContext *context)
{
	g3d_plugins_cleanup(context);

	g_free(context);
}

/* callback wrappers - to be called from libg3d plugins */

gboolean g3d_context_set_bgcolor(G3DContext *context,
	gfloat r, gfloat g, gfloat b, gfloat a)
{
	if(context->set_bgcolor_func)
		return context->set_bgcolor_func(r, g, b, a,
			context->set_bgcolor_data);
	else
		return FALSE;
}

gboolean g3d_context_update_interface(G3DContext *context)
{
	if(context->update_interface_func)
		return context->update_interface_func(context->update_interface_data);
	else
		return FALSE;
}

gboolean g3d_context_update_progress_bar(G3DContext *context,
	gfloat percentage, gboolean visibility)
{
	if(context->update_progress_bar_func)
		return context->update_progress_bar_func(
			percentage, visibility,
			context->update_progress_bar_data);
	else
		return FALSE;
}

/* set callback functions - to be called from client program */

void g3d_context_set_set_bgcolor_func(G3DContext *context,
	G3DSetBgColorFunc func, gpointer user_data)
{
	context->set_bgcolor_func = func;
	context->set_bgcolor_data = user_data;
}

void g3d_context_set_update_interface_func(G3DContext *context,
	G3DUpdateInterfaceFunc func, gpointer user_data)
{
	context->update_interface_func = func;
	context->update_interface_data = user_data;
}

void g3d_context_set_update_progress_bar_func(G3DContext *context,
	G3DUpdateProgressBarFunc func, gpointer user_data)
{
	context->update_progress_bar_func = func;
	context->update_progress_bar_data = user_data;
}
