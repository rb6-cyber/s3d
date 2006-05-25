/* $Id: test_lib.c,v 1.1.2.2 2006/01/23 17:04:37 dahms Exp $ */

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
#include <stdlib.h>

#include <g3d/g3d.h>

int main(int argc, char *argv[])
{
	G3DContext *context;
	G3DModel *model;
	G3DObject *object;
	GSList *oitem;
	guint32 i;

	context = g3d_context_new();

	if(argc > 1)
	{
		model = g3d_model_load(context, argv[1]);

		if(model)
		{
			g_print("%s: %d objects:\n", argv[1],
				g_slist_length(model->objects));

			i = 0;
			oitem = model->objects;
			while(oitem)
			{
				object = (G3DObject *)oitem->data;
				g_print("  [%2u] %-50s (%d faces, %d mats)\n",
					i,
					object->name ? object->name : "(NULL)",
					g_slist_length(object->faces),
					g_slist_length(object->materials));

				oitem = oitem->next;
				i ++;
			}

			g3d_model_free(model);
		}
	}

	g3d_context_free(context);

	return EXIT_SUCCESS;
}
