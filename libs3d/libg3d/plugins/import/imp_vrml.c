/* $Id: imp_vrml.c,v 1.1.2.2 2006/01/23 17:03:06 dahms Exp $ */

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

#include <stdio.h>
#include <string.h>
#include <locale.h>

#include <glib.h>

#include <g3d/types.h>
#include <g3d/material.h>

#include "imp_vrml_v1.h"

#define VRML_FT_VRML      0x01
#define VRML_FT_INVENTOR  0x02

gboolean plugin_load_model(G3DContext *context, const gchar *filename,
	G3DModel *model, gpointer user_data)
{
	FILE *f;
	yyscan_t scanner;
	G3DMaterial *material;
	gchar buffer[128];
	guint32 ver_maj, ver_min, filetype;

	f = fopen(filename, "r");
	if(f == NULL)
	{
		g_warning("failed to read '%s'", filename);
		return FALSE;
	}

	memset(buffer, 0, 128);
	fgets(buffer, 127, f);
	if(strncmp(buffer, "#VRML", 5) == 0)
	{
		filetype = VRML_FT_VRML;
	}
	else if(strncmp(buffer, "#Inventor", 9) == 0)
	{
		filetype = VRML_FT_INVENTOR;
	}
	else
	{
		g_warning("file '%s' is not a VRML file", filename);
		fclose(f);
		return FALSE;
	}

	/* FIXME: more than one space between VRML and Vx.x? */
	ver_maj = buffer[7] - '0';
	ver_min = buffer[9] - '0';

#if DEBUG > 0
	g_print("VRML: version %d.%d\n", ver_maj, ver_min);
#endif

	setlocale(LC_NUMERIC, "C");

	if((filetype == VRML_FT_INVENTOR) || (ver_maj == 1))
	{
		/* Inventor / VRML 1.x */

		material = g3d_material_new();
		material->name = g_strdup("fallback material");
		model->materials = g_slist_append(model->materials, material);

		vrml_v1_yylex_init(&scanner);
		vrml_v1_yyset_in(f, scanner);
		vrml_v1_yyset_extra(model, scanner);
		vrml_v1_yylex(scanner);
		vrml_v1_yylex_destroy(scanner);
	}
	else if(ver_maj == 2)
	{
		g_warning("VRML 2 is not yet supported");
		fclose(f);
		return FALSE;
	}
	else
	{
		g_warning("unknown VRML version %d.%d", ver_maj, ver_min);
	}

	fclose(f);
	return TRUE;
}

char *plugin_description(void)
{
	return g_strdup("import plugin for VRML 2.0+ files\n");
}

char **plugin_extensions(void)
{
	return g_strsplit("wrl:vrml:iv", ":", 0);
}

/* evil hack [tm] */

extern int yywrap(yyscan_t yyscanner);

int vrml_v1_yywrap(yyscan_t yyscanner)
{
	return yywrap(yyscanner);
}

/* VRML specific */

