/* $Id: primitive.c,v 1.1.2.3 2006/01/23 16:38:47 dahms Exp $ */

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

#include <math.h>

#ifndef M_PI
#	define M_PI 3.14159265358979323846
#endif

#include <g3d/types.h>
#include <g3d/vector.h>

G3DObject *g3d_primitive_cylinder(gfloat radius, gfloat height,
	guint32 sides, gboolean top, gboolean bottom, G3DMaterial *material)
{
	G3DObject *object;
	G3DFace *face;
	guint32 i;

	if(sides < 3)
		return FALSE;

	object = g_new0(G3DObject, 1);

	/* vertices */
	object->vertex_count = sides * 2 + 2;
	object->vertex_data = g_new0(gfloat, object->vertex_count * 3);

	/* 2 rings */
	for(i = 0; i < sides; i ++)
	{
		object->vertex_data[i * 3 + 0] =
		object->vertex_data[(sides + i) * 3 + 0] =
			radius * cos(M_PI * 2 * i / sides);
		object->vertex_data[i * 3 + 1] =
		object->vertex_data[(sides + i) * 3 + 1] =
			radius * sin(M_PI * 2 * i / sides);

		object->vertex_data[i * 3 + 2] = 0.0;
		object->vertex_data[(sides + i) * 3 + 2] = height;
	}

	/* center top & bottom */
	object->vertex_data[sides * 2 * 3 + 0] = 0.0;
	object->vertex_data[sides * 2 * 3 + 1] = 0.0;
	object->vertex_data[sides * 2 * 3 + 2] = 0.0;

	object->vertex_data[sides * 2 * 3 + 3] = 0.0;
	object->vertex_data[sides * 2 * 3 + 4] = 0.0;
	object->vertex_data[sides * 2 * 3 + 5] = height;

	/* ring faces */
	for(i = 0; i < sides; i ++)
	{
		face = g_new0(G3DFace, 1);
		face->material = material;
		face->vertex_count = 4;
		face->vertex_indices = g_new0(guint32, 4);

		face->vertex_indices[0] = i;
		face->vertex_indices[1] = i + sides;

		if(i == (sides - 1))
		{
			face->vertex_indices[2] = sides;
			face->vertex_indices[3] = 0;
		}
		else
		{
			face->vertex_indices[2] = i + sides + 1;
			face->vertex_indices[3] = i + 1;
		}

		/* normals */
		face->flags |= G3D_FLAG_FAC_NORMALS;
		face->normals = g_new0(gfloat, 4 * 3);

		face->normals[0 * 3 + 0] =
		face->normals[1 * 3 + 0] =
			object->vertex_data[i * 3 + 0];
		face->normals[0 * 3 + 1] =
		face->normals[1 * 3 + 1] =
			object->vertex_data[i * 3 + 1];
		face->normals[0 * 3 + 2] = 0.0;
		face->normals[1 * 3 + 2] = 0.0;

		g3d_vector_unify(
			&(face->normals[0 * 3 + 0]),
			&(face->normals[0 * 3 + 1]),
			&(face->normals[0 * 3 + 2]));
		g3d_vector_unify(
			&(face->normals[1 * 3 + 0]),
			&(face->normals[1 * 3 + 1]),
			&(face->normals[1 * 3 + 2]));

		face->normals[2 * 3 + 0] =
		face->normals[3 * 3 + 0] =
			object->vertex_data[face->vertex_indices[2] * 3 + 0];
		face->normals[2 * 3 + 1] =
		face->normals[3 * 3 + 1] =
			object->vertex_data[face->vertex_indices[2] * 3 + 1];
		face->normals[2 * 3 + 2] = 0.0;
		face->normals[3 * 3 + 2] = 0.0;

		g3d_vector_unify(
			&(face->normals[2 * 3 + 0]),
			&(face->normals[2 * 3 + 1]),
			&(face->normals[2 * 3 + 2]));
		g3d_vector_unify(
			&(face->normals[3 * 3 + 0]),
			&(face->normals[3 * 3 + 1]),
			&(face->normals[3 * 3 + 2]));

		object->faces = g_slist_append(object->faces, face);
	}

	if(top)
	{
		for(i = 0; i < sides; i ++)
		{
			face = g_new0(G3DFace, 1);
			face->material = material;
			face->vertex_count = 3;
			face->vertex_indices = g_new0(guint32, 3);

			face->vertex_indices[0] = sides + i;
			face->vertex_indices[1] = sides * 2 + 1; /* top center */
			if(i == (sides - 1))
				face->vertex_indices[2] = sides;
			else
				face->vertex_indices[2] = sides + i + 1;

			object->faces = g_slist_append(object->faces, face);
		}
	}

	if(bottom)
	{
		for(i = 0; i < sides; i ++)
		{
			face = g_new0(G3DFace, 1);
			face->material = material;
			face->vertex_count = 3;
			face->vertex_indices = g_new0(guint32, 3);

			face->vertex_indices[0] = i;
			face->vertex_indices[1] = sides * 2; /* bottom center */
			if(i == (sides - 1))
				face->vertex_indices[2] = 0;
			else
				face->vertex_indices[2] = i + 1;

			object->faces = g_slist_append(object->faces, face);
		}
	}

	return object;
}

G3DObject *g3d_primitive_tube(gfloat r_in, gfloat r_out, gfloat height,
	guint32 sides, gboolean top, gboolean bottom, G3DMaterial *material)
{
	G3DObject *object;
	G3DFace *face;
	guint32 i, j;

	if(sides < 3)
		return NULL;

	object = g_new0(G3DObject, 1);

	/* vertices */
	object->vertex_count = sides * 4;
	object->vertex_data = g_new0(gfloat, object->vertex_count * 3);

	/*
	 * outer lower     0
	 * outer upper     sides
	 * inner lower     sides * 2
	 * inner upper     sides * 3
	 */

	/* 4 rings */
	for(i = 0; i < sides; i ++)
	{
		/* outer rings */
		object->vertex_data[i * 3 + 0] =
		object->vertex_data[(sides + i) * 3 + 0] =
			r_out * cos(M_PI * 2 * i / sides);
		object->vertex_data[i * 3 + 1] =
		object->vertex_data[(sides + i) * 3 + 1] =
			r_out * sin(M_PI * 2 * i / sides);

		object->vertex_data[i * 3 + 2] = 0.0;
		object->vertex_data[(sides + i) * 3 + 2] = height;

		/* inner rings */
		object->vertex_data[(sides * 2 + i) * 3 + 0] =
		object->vertex_data[(sides * 3 + i) * 3 + 0] =
			r_in * cos(M_PI * 2 * i / sides);
		object->vertex_data[(sides * 2 + i) * 3 + 1] =
		object->vertex_data[(sides * 3 + i) * 3 + 1] =
			r_in * sin(M_PI * 2 * i / sides);

		object->vertex_data[(sides * 2 + i) * 3 + 2] = 0.0;
		object->vertex_data[(sides * 3 + i) * 3 + 2] = height;
	}

	/* ring faces */
	for(i = 0; i < sides; i ++)
	{
		/* outer and inner faces */
		for(j = 0; j < 2; j ++)
		{
			face = g_new0(G3DFace, 1);
			face->material = material;
			face->vertex_count = 4;
			face->vertex_indices = g_new0(guint32, 4);

			face->vertex_indices[0] = i + sides * (j * 2 + 0);
			face->vertex_indices[1] = i + sides * (j * 2 + 1);

			if(i == (sides - 1))
			{
				face->vertex_indices[2] = sides * (j * 2 + 1);
				face->vertex_indices[3] = sides * (j * 2 + 0);
			}
			else
			{
				face->vertex_indices[2] = i + sides * (j * 2 + 1) + 1;
				face->vertex_indices[3] = i + sides * (j * 2 + 0) + 1;
			}

			/* normals */
			face->flags |= G3D_FLAG_FAC_NORMALS;
			face->normals = g_new0(gfloat, 4 * 3);

			face->normals[0 * 3 + 0] =
			face->normals[1 * 3 + 0] =
				object->vertex_data[face->vertex_indices[0] * 3 + 0] *
				(j ? -1 : 1);
			face->normals[0 * 3 + 1] =
			face->normals[1 * 3 + 1] =
				object->vertex_data[face->vertex_indices[0] * 3 + 1] *
				(j ? -1 : 1);
			face->normals[0 * 3 + 2] = 0.0;
			face->normals[1 * 3 + 2] = 0.0;

			g3d_vector_unify(
				&(face->normals[0 * 3 + 0]),
				&(face->normals[0 * 3 + 1]),
				&(face->normals[0 * 3 + 2]));
			g3d_vector_unify(
				&(face->normals[1 * 3 + 0]),
				&(face->normals[1 * 3 + 1]),
				&(face->normals[1 * 3 + 2]));

			face->normals[2 * 3 + 0] =
			face->normals[3 * 3 + 0] =
				object->vertex_data[face->vertex_indices[2] * 3 + 0] *
				(j ? -1 : 1);
			face->normals[2 * 3 + 1] =
			face->normals[3 * 3 + 1] =
				object->vertex_data[face->vertex_indices[2] * 3 + 1] *
				(j ? -1 : 1);
			face->normals[2 * 3 + 2] = 0.0;
			face->normals[3 * 3 + 2] = 0.0;

			g3d_vector_unify(
				&(face->normals[2 * 3 + 0]),
				&(face->normals[2 * 3 + 1]),
				&(face->normals[2 * 3 + 2]));
			g3d_vector_unify(
				&(face->normals[3 * 3 + 0]),
				&(face->normals[3 * 3 + 1]),
				&(face->normals[3 * 3 + 2]));

			object->faces = g_slist_append(object->faces, face);
		}
	}

	/* top/bottom faces if requested */
	for(i = 0; i < sides; i ++)
	{
		for(j = (bottom ? 0 : 1); j < (top ? 2 : 1); j ++)
		{
			face = g_new0(G3DFace, 1);
			face->material = material;
			face->vertex_count = 4;
			face->vertex_indices = g_new0(guint32, 4);

			face->vertex_indices[0] = sides * (2 + j) + i; /* inner */
			face->vertex_indices[1] = sides * (0 + j) + i; /* outer */

			if(i == (sides - 1))
			{
				face->vertex_indices[2] = sides * (0 + j); /* outer first */
				face->vertex_indices[3] = sides * (2 + j); /* inner first */
			}
			else
			{
				face->vertex_indices[2] = sides * (0 + j) + i + 1;
				face->vertex_indices[3] = sides * (2 + j) + i + 1;
			}

			object->faces = g_slist_append(object->faces, face);
		}
	}

	return object;
}
