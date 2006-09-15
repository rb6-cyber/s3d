
#include <string.h>

#include <glib.h>

#include <g3d/context.h>
#include <g3d/material.h>
#include <g3d/texture.h>
#include <g3d/iff.h>
#include <g3d/read.h>

#include "imp_lwo.h"

gboolean lwo_cb_CLIP(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	LwoObject *obj;
	guint32 index;

	obj = (LwoObject *)global->user_data;
	g_return_val_if_fail(obj != NULL, FALSE);

	if(!local->finalize)
	{
		index = g3d_read_int32_be(global->f);
		local->nb -= 4;

		obj->nclips ++;
		obj->clips = g_realloc(obj->clips, obj->nclips * sizeof(guint32));
		obj->clipfiles = g_realloc(obj->clipfiles,
			(obj->nclips + 1) * sizeof(gchar *));

		obj->clips[obj->nclips - 1] = index;
		obj->clipfiles[obj->nclips - 1] = g_strdup("undef");
		obj->clipfiles[obj->nclips] = NULL;
	}

	return TRUE;
}

gboolean lwo_cb_COLR(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	G3DMaterial *material;

	material = (G3DMaterial *)local->object;
	g_return_val_if_fail(material != NULL, FALSE);

	if(global->flags & LWO_FLAG_LWO2)
	{
		material->r = g3d_read_float_be(global->f);
		material->g = g3d_read_float_be(global->f);
		material->b = g3d_read_float_be(global->f);
		local->nb -= 12;
		g3d_read_int16_be(global->f);
		local->nb -= 2;
	}
	else
	{
		material->r = g3d_read_int8(global->f) / 255.0;
		material->g = g3d_read_int8(global->f) / 255.0;
		material->b = g3d_read_int8(global->f) / 255.0;
		g3d_read_int8(global->f);
		local->nb -= 4;
	}

	return TRUE;
}

/* image index */
gboolean lwo_cb_IMAG(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	LwoObject *obj;
	G3DMaterial *material;
	guint32 index, i;

	obj = (LwoObject *)global->user_data;
	g_return_val_if_fail(obj != NULL, FALSE);

	material = (G3DMaterial *)local->object;
	g_return_val_if_fail(material != NULL, FALSE);

	local->nb -= lwo_read_vx(global->f, &index);

	for(i = 0; i < obj->nclips; i ++)
	{
		if(obj->clips[i] == index)
			break;
	}

	if(obj->clips[i] == index)
	{
		material->tex_image = g3d_texture_load_cached(
			global->context, global->model, obj->clipfiles[i]);
	}

	return TRUE;
}

/* points */
gboolean lwo_cb_PNTS(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	LwoObject *obj;
	G3DObject *object;
	gint32 i, off;

	obj = (LwoObject *)global->user_data;
	g_return_val_if_fail(obj != NULL, FALSE);

	if(global->flags & LWO_FLAG_LWO2)
	{
		object = lwo_create_object(global->f, global->model, global->flags);
		obj->object = object;

		if(obj->tex_vertices)
		{
			g_free(obj->tex_vertices);
			obj->tex_vertices = NULL;
		}
	}
	else
	{
		object = (G3DObject *)obj->object;
		if(object == NULL)
		{
			object = lwo_create_object(global->f, global->model,
				global->flags);
			obj->object = object;
		}
	}

	off = object->vertex_count;
	object->vertex_count += (local->nb / 12);
	g_return_val_if_fail(object->vertex_count >= 3, FALSE);

	object->vertex_data = g_realloc(object->vertex_data,
		sizeof(gfloat) * object->vertex_count * 3);

	for(i = off; i < object->vertex_count; i ++)
	{
		object->vertex_data[i * 3 + 0] = g3d_read_float_be(global->f);
		object->vertex_data[i * 3 + 1] = g3d_read_float_be(global->f);
		object->vertex_data[i * 3 + 2] = g3d_read_float_be(global->f);
		local->nb -= 12;
	}

	return TRUE;
}

/* polygons */
gboolean lwo_cb_POLS(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	LwoObject *obj;
	G3DObject *object;
	G3DFace *face;
	guint32 type;
	gint32 n = 0, i, nmat, det_cnt, cnt;
	gint16 i16;
	gchar *tmp;

	obj = (LwoObject *)global->user_data;
	g_return_val_if_fail(obj != NULL, FALSE);

	object = (G3DObject *)obj->object;
	g_return_val_if_fail(object != NULL, FALSE);

	if(global->flags & LWO_FLAG_LWO2)
	{
		type = g3d_read_int32_be(global->f);
		local->nb -= 4;

		if(type != G3D_IFF_MKID('F', 'A', 'C', 'E'))
		{
			tmp = g3d_iff_id_to_text(type);
			g_warning("[LWO] unhandled polygon type %s", tmp);
			g_free(tmp);
			return FALSE;
		}
	}

	while(local->nb > 0)
	{
		n ++;
		face = g_new0(G3DFace, 1);
		face->vertex_count = g3d_read_int16_be(global->f);
		local->nb -= 2;

		if(global->flags & LWO_FLAG_LWO2)
			face->vertex_count &= 0x03FF;

		face->vertex_indices = g_new0(guint32, face->vertex_count);

		if(obj->tex_vertices)
		{
			face->flags |= G3D_FLAG_FAC_TEXMAP;
			face->tex_vertex_count = face->vertex_count;
			face->tex_vertex_data = g_new0(gfloat, face->tex_vertex_count * 2);
		}

		for(i = 0; i < face->vertex_count; i ++)
		{
			if(global->flags & LWO_FLAG_LWO2)
			{
				local->nb -= lwo_read_vx(global->f,
					&(face->vertex_indices[i]));
			}
			else
			{
				face->vertex_indices[i] = g3d_read_int16_be(global->f);
				local->nb -= 2;
#if 0
				i16 = g3d_read_int16_be(global->f);
				local->nb -= 2;

				if(i16 < 0)
					face->vertex_indices[i] = - i16;
				else
					face->vertex_indices[i] = i16;
#endif

				if(face->vertex_indices[i] > object->vertex_count)
					face->vertex_indices[i] = 0;
			}

			if(obj->tex_vertices)
			{
				face->tex_vertex_data[i * 2 + 0] =
					obj->tex_vertices[face->vertex_indices[i] * 2 + 0];
				face->tex_vertex_data[i * 2 + 1] =
					obj->tex_vertices[face->vertex_indices[i] * 2 + 1];
			}
		} /* i: 0..face->vertex_count */

		if(!(global->flags & LWO_FLAG_LWO2))
		{
			nmat = g3d_read_int16_be(global->f);
			local->nb -= 2;

			if(nmat < 0)
			{
				/* detail polygons, skipped */
				det_cnt = g3d_read_int16_be(global->f);
				local->nb -= 2;
				nmat *= -1;
				while(det_cnt-- > 0)
				{
					cnt = g3d_read_int16_be(global->f);
					local->nb -= 2;

					fseek(global->f, cnt * 2 + 2, SEEK_CUR);
					local->nb -= cnt * 2 + 2;
				}
			}
			else if(nmat == 0)
			{
				nmat = 1;
			}

			face->material = g_slist_nth_data(global->model->materials, nmat);

			if(face->material == NULL)
			{
#if 0
				g_warning("[LWO] face->material is NULL (#%d)\n", nmat - 1);
#endif
				face->material = g_slist_nth_data(global->model->materials, 0);
			}

		} /* !LWO2 */
		else
		{
			face->material = g_slist_nth_data(global->model->materials, 0);
		} /* LWO2 */

		if(face->vertex_count < 3)
		{
			g_free(face->vertex_indices);
			g_free(face);
		}
		else
		{
			object->faces = g_slist_prepend(object->faces, face);
		}

		g3d_context_update_interface(global->context);
	} /* local->nb > 0 */

	return TRUE;
}

/* poly tag mapping */
gboolean lwo_cb_PTAG(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	LwoObject *obj;
	G3DObject *object;
	G3DMaterial *material, *tmat;
	G3DFace *face;
	GSList *mlist;
	gint32 id, fmax;
	guint32 poly, tag;

	obj = (LwoObject *)global->user_data;
	g_return_val_if_fail(obj != NULL, FALSE);

	object = (G3DObject *)obj->object;
	g_return_val_if_fail(object != NULL, FALSE);

	id = g3d_read_int32_be(global->f);
	local->nb -= 4;

	if(id != G3D_IFF_MKID('S','U','R','F'))
		return FALSE;

	fmax = g_slist_length(object->faces) - 1;

	while(local->nb > 0)
	{
		local->nb -= lwo_read_vx(global->f, &poly);
		tag = g3d_read_int16_be(global->f);
		local->nb -= 2;

		face = (G3DFace *)g_slist_nth_data(object->faces, fmax - poly);
		g_return_val_if_fail(face != NULL, FALSE);

		if(tag > obj->ntags)
		{
			g_printerr("[LWO] tag %d not listed (%d tags)\n", tag, obj->ntags);
			continue;
		}

		material = NULL;
		mlist = global->model->materials;
		while(mlist != NULL)
		{
			tmat = (G3DMaterial*)mlist->data;
			if(strcmp(obj->tags[tag], tmat->name) == 0)
			{
				material = tmat;
				break;
			}
			mlist = mlist->next;
		}

		if(material)
			face->material = material;
		else
			g_printerr("[LWO] unknown material tag %s\n", obj->tags[tag]);
	}

	return TRUE;
}

/* specularity */
gboolean lwo_cb_SPEC(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	G3DMaterial *material;
	gfloat tmpf;

	material = (G3DMaterial *)local->object;
	g_return_val_if_fail(material != NULL, FALSE);

	if(global->flags & LWO_FLAG_LWO2)
	{
		tmpf = 1.0 - g3d_read_float_be(global->f);
		local->nb -= 4;
	}
	else
	{
		tmpf = 1.0 - (gfloat)g3d_read_int16_be(global->f) / 256.0;
		local->nb -= 2;
	}

	material->specular[0] = material->r * tmpf;
	material->specular[1] = material->g * tmpf;
	material->specular[2] = material->b * tmpf;

	return TRUE;
}

/* surfaces */
gboolean lwo_cb_SRFS(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	LwoObject *obj;
	G3DMaterial *material;
	gchar buffer[512];

	obj = (LwoObject *)global->user_data;
	g_return_val_if_fail(obj != NULL, FALSE);

	while(local->nb > 0)
	{
		material = g3d_material_new();
		local->nb -= lwo_read_string(global->f, buffer);
		material->name = g_strdup(buffer);
		global->model->materials = g_slist_append(global->model->materials,
			material);

	}

	return TRUE;
}

/* still image */
gboolean lwo_cb_STIL(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	LwoObject *obj;

	gchar buffer[512];

    obj = (LwoObject *)global->user_data;
	g_return_val_if_fail(obj != NULL, FALSE);

	local->nb -= lwo_read_string(global->f, buffer);

	g_free(obj->clipfiles[obj->nclips - 1]);
	obj->clipfiles[obj->nclips - 1] = g_strdup(buffer);
	obj->clipfiles[obj->nclips] = NULL;

	return TRUE;
}

/* surface */
gboolean lwo_cb_SURF(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	LwoObject *obj;
	G3DObject *object;
	G3DMaterial *material = NULL, *tmat;
	GSList *mlist;
	gchar name[512];

	obj = (LwoObject *)global->user_data;
	g_return_val_if_fail(obj != NULL, FALSE);

	object = (G3DObject *)obj->object;
	g_return_val_if_fail(object != NULL, FALSE);

	if(!local->finalize)
	{
		local->nb -= lwo_read_string(global->f, name);

		if(global->flags & LWO_FLAG_LWO2)
		{
			g3d_read_int16_be(global->f);
			local->nb -= 2;
		}

		mlist = global->model->materials;
		while(mlist != NULL)
		{
			tmat = (G3DMaterial*)mlist->data;
			if(strcmp(name, tmat->name) == 0)
			{
				material = tmat;
				break;
			}
			mlist = mlist->next;
		}

		if(material == NULL)
		{
			material = g3d_material_new();
			material->name = g_strdup(name);
			global->model->materials = g_slist_append(global->model->materials,
				material);
		}

		local->object = material;
	}

	return TRUE;
}

/* tags */
gboolean lwo_cb_TAGS(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	LwoObject *obj;
	G3DMaterial *material;
	gchar buffer[512];

	obj = (LwoObject *)global->user_data;
	g_return_val_if_fail(obj != NULL, FALSE);

	if(obj->ntags)
	{
		g_strfreev(obj->tags);
		obj->ntags = 0;
	}

	/* read tags */
	while(local->nb > 0)
	{
		local->nb -= lwo_read_string(global->f, buffer);
		obj->ntags ++;
		obj->tags = g_realloc(obj->tags, (1 + obj->ntags) * sizeof(gchar *));
		obj->tags[obj->ntags - 1] = g_strdup(buffer);
		obj->tags[obj->ntags] = NULL;

		material = g3d_material_new();
		material->name = g_strdup(buffer);
		global->model->materials = g_slist_append(global->model->materials,
			material);
	}

	return TRUE;
}

/* transparency */
gboolean lwo_cb_TRAN(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	G3DMaterial *material;

	material = (G3DMaterial *)local->object;
	g_return_val_if_fail(material != NULL, FALSE);

	if(global->flags & LWO_FLAG_LWO2)
	{
		material->a = 1.0 - g3d_read_float_be(global->f);
		local->nb -= 4;
	}
	else
	{
		material->a = 1.0 - (gfloat)g3d_read_int16_be(global->f) / 256.0;
		local->nb -= 2;
	}

	if(material->a < 0.1)
		material->a = 0.1;

	return TRUE;
}

/* vertex mapping */
gboolean lwo_cb_VMAP(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	LwoObject *obj;
	guint32 index, type, dim;
	gchar buffer[512], *tmp;

	obj = (LwoObject *)global->user_data;
	g_return_val_if_fail(obj != NULL, FALSE);

	tmp = g3d_iff_id_to_text(local->parent_id);
	g_debug("[LWO][VMAP] parent is %s", tmp);
	g_free(tmp);

	if(local->parent_id == G3D_IFF_MKID('L','W','O','2'))
	{
		type = g3d_read_int32_be(global->f);
		local->nb -= 4;

		dim = g3d_read_int16_be(global->f);
		local->nb -= 2;

		local->nb -= lwo_read_string(global->f, buffer);

		if(type == G3D_IFF_MKID('T','X','U','V'))
		{
			g_debug("[LWO][VMAP] **TXUV**");

			g_return_val_if_fail(obj->tex_vertices == NULL, FALSE);

			obj->tex_vertices = g_new0(gfloat,
				obj->object->vertex_count * 2);

			while(local->nb > 0)
			{
				local->nb -= lwo_read_vx(global->f, &index);
				g_return_val_if_fail(index < obj->object->vertex_count, FALSE);

				obj->tex_vertices[index * 2 + 0] =
					g3d_read_float_be(global->f);
				obj->tex_vertices[index * 2 + 1] =
					1.0 - g3d_read_float_be(global->f);
				local->nb -= 8;
			}
		}
		else
		{
			tmp = g3d_iff_id_to_text(type);
			g_warning("[LWO][VMAP] unhandled vertex mapping %s", tmp);
			g_free(tmp);
		}
	}

	return TRUE;
}
