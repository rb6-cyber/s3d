#include <string.h>

#include <g3d/read.h>
#include <g3d/texture.h>
#include <g3d/vector.h>
#include <g3d/matrix.h>

#include "imp_3ds_callbacks.h"

#define X3DS_FLAG_TENSION       0x01
#define X3DS_FLAG_CONTINUITY    0x02
#define X3DS_FLAG_BIAS          0x04
#define X3DS_FLAG_EASE_TO       0x08
#define X3DS_FLAG_EASE_FROM     0x10

gboolean x3ds_cb_0x0002(x3ds_global_data *global, x3ds_parent_data *parent)
{
	gint32 version;

	version = g3d_read_int32_le(global->f);
	parent->nb -= 4;
#if DEBUG > 0
	g_printerr("[3DS] M3D version %d\n", version);
#endif
	return TRUE;
}

/* color float */
gboolean x3ds_cb_0x0010(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DMaterial *material;
	gfloat r, g, b;

	r = g3d_read_float_le(global->f);
	g = g3d_read_float_le(global->f);
	b = g3d_read_float_le(global->f);
	parent->nb -= 12;

	switch(parent->id)
	{
		case 0x1200: /* SOLID_BGND */
			g3d_context_set_bgcolor(global->context, r, g, b, 1.0);
			break;

		case 0xA010: /* ambient color */
		case 0xA020: /* diffuse color */
			material = (G3DMaterial *)parent->object;
			g_return_val_if_fail(material, FALSE);

			material->r = r;
			material->g = g;
			material->b = b;
			break;

		case 0xA030: /* specular color */
			material = (G3DMaterial *)parent->object;
			g_return_val_if_fail(material, FALSE);

			material->specular[0] = r;
			material->specular[1] = g;
			material->specular[2] = b;
			material->specular[3] = 0.25;
			break;

		default:
			g_printerr("[3DS] unhandled COLOR_F in 0x%04X\n", parent->id);
			break;
	}

	return TRUE;
}

/* color 24 */
gboolean x3ds_cb_0x0011(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DMaterial *material;
	gint32 r, g, b;

	material = (G3DMaterial *)parent->object;
	g_return_val_if_fail(material, FALSE);

	r = g3d_read_int8(global->f);
	g = g3d_read_int8(global->f);
	b = g3d_read_int8(global->f);
	parent->nb -= 3;

	switch(parent->id)
	{
#if 0
		case 0xA010: /* ambient color */
#endif
		case 0xA020: /* diffuse color */
			material->r = (gfloat)r / 255.0;
			material->g = (gfloat)g / 255.0;
			material->b = (gfloat)b / 255.0;
			break;

		case 0xA030: /* specular color */
			material->specular[0] = (gfloat)r / 255.0;
			material->specular[1] = (gfloat)g / 255.0;
			material->specular[2] = (gfloat)b / 255.0;
			material->specular[3] = 0.25;
			break;

		default:
			g_printerr("[3DS] unhandled COLOR_24 in 0x%04X\n", parent->id);
			break;
	}

	return TRUE;
}

/* short percentage */
gboolean x3ds_cb_0x0030(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DMaterial *material;
	gint32 percent;

	material = (G3DMaterial *)parent->object;
	g_return_val_if_fail(material, FALSE);

	percent = g3d_read_int16_le(global->f);
	parent->nb -= 2;

	switch(parent->id)
	{
		case 0xA040: /* shininess */
			material->shininess = (gfloat)percent / 100.0;
			break;

		case 0xA041: /* shininess (2) */
			/* TODO: do something here? */
			break;

		case 0xA050: /* transparency */
			material->a = 1.0 - ((gfloat)percent / 100.0);
			break;

		case 0xA052: /* fallthrough */
			/* TODO: do something here? */
			break;

		case 0xA053: /* blur */
			/* TODO: do something here? */
			break;

		case 0xA084: /* self illumination */
			/* TODO: do something here? */
			break;

		case 0xA200: /* texture map */
			/* TODO: do something here? */
			break;

		case 0xA210: /* opacity map */
			/* TODO: do something here? */
			g_printerr("[3DS] opacity percentage: %d%%\n", percent);
			break;

		case 0xA220: /* reflection map */
			/* TODO: do something here? */
			break;

		case 0xA230: /* bump map */
			/* TODO: do something here? */
			break;

		default:
			g_printerr("[3DS] unhandled INT_PERCENTAGE in 0x%04X\n",
				parent->id);
			break;
	}

	return TRUE;
}

/* float percentage */
gboolean x3ds_cb_0x0031(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DMaterial *material;
	gfloat percent;

	material = (G3DMaterial *)parent->object;
	g_return_val_if_fail(material, FALSE);

	percent = g3d_read_float_le(global->f);
	parent->nb -= 4;

	switch(parent->id)
	{
		case 0xA040: /* shininess */
			material->shininess = percent;
			break;

		case 0xA050: /* transparency */
			material->a = 1.0 - percent;
			break;

		default:
			g_printerr("[3DS] unhandled FLOAT_PERCENTAGE in 0x%04X\n",
				parent->id);
			break;
	}

	return TRUE;
}

/* master scale */
gboolean x3ds_cb_0x0100(x3ds_global_data *global, x3ds_parent_data *parent)
{
	global->scale = g3d_read_float_le(global->f);
	parent->nb -= 4;

	return TRUE;
}

/* named object */
gboolean x3ds_cb_0x4000(x3ds_global_data *global, x3ds_parent_data *parent)
{
	gchar buffer[1024];

	parent->nb -= x3ds_read_cstr(global->f, buffer);
	parent->object = x3ds_newobject(global->model, buffer);

	return TRUE;
}

/* point array */
gboolean x3ds_cb_0x4110(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DObject *object;
	gint32 i;

	object = (G3DObject *)parent->object;
	g_return_val_if_fail(object, FALSE);

	object->vertex_count = g3d_read_int16_le(global->f);
	parent->nb -= 2;

	object->vertex_data = g_new0(gfloat, object->vertex_count * 3);
	for(i = 0; i < object->vertex_count; i ++)
	{
		object->vertex_data[i * 3 + 0] = g3d_read_float_le(global->f);
		object->vertex_data[i * 3 + 1] = g3d_read_float_le(global->f);
		object->vertex_data[i * 3 + 2] = g3d_read_float_le(global->f);
		parent->nb -= 12;

		if((i % 1000) == 0) x3ds_update_progress(global);
	}
	return TRUE;
}

/* face array */
gboolean x3ds_cb_0x4120(x3ds_global_data *global, x3ds_parent_data *parent)
{
	gint32 i, flags, nfaces;
#define X3DS_REORDER_FACES
#ifdef X3DS_REORDER_FACES
	gint32 p1 = -1, p2 = -1, bottle;
#endif
	G3DFace *face;
	G3DObject *object;

	object = (G3DObject *)parent->object;
	g_return_val_if_fail(object, FALSE);

	nfaces = g3d_read_int16_le(global->f);
	parent->nb -= 2;

	for(i = 0; i < nfaces; i ++)
	{
		face = g_new0(G3DFace, 1);

		face->vertex_count = 3;
		face->vertex_indices = g_malloc(3 * sizeof(guint32));

		face->vertex_indices[0] = g3d_read_int16_le(global->f);
		face->vertex_indices[1] = g3d_read_int16_le(global->f);
		face->vertex_indices[2] = g3d_read_int16_le(global->f);
		flags = g3d_read_int16_le(global->f);
		parent->nb -= 8;

#ifdef X3DS_REORDER_FACES
		/* try to put all faces in the same direction */
		if((p1 == face->vertex_indices[0]) && (p2 == face->vertex_indices[1]))
		{
			bottle = face->vertex_indices[0];
			face->vertex_indices[0] = face->vertex_indices[2];
			face->vertex_indices[2] = bottle;
		}

		p1 = face->vertex_indices[0];
		p2 = face->vertex_indices[1];
#endif

		face->material = g_slist_nth_data(object->materials, 0);

		object->faces = g_slist_append(object->faces, face);

		if((i % 1000) == 0) x3ds_update_progress(global);
	}

	return TRUE;
}

/* mesh mat group */
gboolean x3ds_cb_0x4130(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DObject *object;
	gint32 i, j, facenum, nfaces;
	gchar buffer[512];
	G3DMaterial *material = NULL, *mat;
	G3DFace *face;
	GSList *mlist;

	object = (G3DObject *)parent->object;
	g_return_val_if_fail(object, FALSE);

	/* name of material */
	parent->nb -= x3ds_read_cstr(global->f, buffer);

	/* find material in list */
	mlist = global->model->materials;
	while(mlist != NULL)
	{
		mat = (G3DMaterial*)mlist->data;
		if(strcmp(mat->name, buffer) == 0)
		{
			material = mat;
			break;
		}
		mlist = mlist->next;
	}

	nfaces = g3d_read_int16_le(global->f);
	parent->nb -= 2;

	for(i = 0; i < nfaces; i ++)
	{
		facenum = g3d_read_int16_le(global->f);
		parent->nb -= 2;

		if(material != NULL)
		{
			face = (G3DFace*)g_slist_nth_data(object->faces, facenum);
			if(face == NULL) continue;

			face->material = material;

			if(face->material->tex_image && object->tex_vertex_data)
			{
				face->flags |= G3D_FLAG_FAC_TEXMAP;
				face->tex_image = face->material->tex_image;
				face->tex_vertex_count = 3;
				face->tex_vertex_data = g_new0(gfloat, 6);
				for(j = 0; j < 3; j ++)
				{
					face->tex_vertex_data[j * 2 + 0] = object->tex_vertex_data[
						face->vertex_indices[j] * 2 + 0];
					face->tex_vertex_data[j * 2 + 1] = object->tex_vertex_data[
						face->vertex_indices[j] * 2 + 1];
				}
			} /* textured face */
		} /* material != NULL */

		if((i % 1000) == 0) x3ds_update_progress(global);
	} /* 0..nfaces */

	return TRUE;
}

/* texture vertices */
gboolean x3ds_cb_0x4140(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DObject *object;
	gint32 i;

	object = (G3DObject *)parent->object;
	g_return_val_if_fail(object, FALSE);

	object->tex_vertex_count = g3d_read_int16_le(global->f);
	parent->nb -= 2;

	object->tex_vertex_data = g_new0(gfloat, object->tex_vertex_count * 2);

	for(i = 0; i < object->tex_vertex_count; i ++)
	{
		object->tex_vertex_data[i * 2 + 0] = g3d_read_float_le(global->f);
		object->tex_vertex_data[i * 2 + 1] = g3d_read_float_le(global->f);
		parent->nb -= 8;

		if((i % 1000) == 0) x3ds_update_progress(global);
	}

	return TRUE;
}

/* texture vertices */
gboolean x3ds_cb_0x4150(x3ds_global_data *global, x3ds_parent_data *parent)
{
// 	G3DObject *object;
// 	gint32 i;
//
// 	object = (G3DObject *)parent->object;
// 	g_return_val_if_fail(object, FALSE);
//
// 	object->tex_vertex_count = g3d_read_int16_le(global->f);
// 	parent->nb -= 2;
//
// 	object->tex_vertex_data = g_new0(gfloat, object->tex_vertex_count * 2);
//
// 	for(i = 0; i < object->tex_vertex_count; i ++)
// 	{
// 		object->tex_vertex_data[i * 2 + 0] = g3d_read_float_le(global->f);
// 		object->tex_vertex_data[i * 2 + 1] = g3d_read_float_le(global->f);
// 		parent->nb -= 8;
//
// 		if((i % 1000) == 0) x3ds_update_progress(global);
// 	}

	return TRUE;
}

/* mesh matrix */
gboolean x3ds_cb_0x4160(x3ds_global_data *global, x3ds_parent_data *parent)
{
	gfloat matrix[16];
	gint32 i;

	g3d_matrix_identity(matrix);
	for(i = 0; i < 12; i ++)
		matrix[i] = g3d_read_float_le(global->f);

#if 0
	for(w = 0; w < 4; w ++)
	{
		for(h = 0; h < 3; h ++)
		{
			matrix[w * 4 + h] = g3d_read_float_le(global->f);
		}
	}
#endif

	parent->nb -= 48;

/* #define X3DS_MESH_TRANSFORM */
#ifdef X3DS_MESH_TRANSFORM
	if(parent->object)
	{
		gint32 i;
		G3DObject *object = (G3DObject *)parent->object;

		for(i = 0; i < object->vertex_count; i ++)
		{
			g3d_vector_transform(
				&(object->vertex_data[i * 3 + 0]),
				&(object->vertex_data[i * 3 + 1]),
				&(object->vertex_data[i * 3 + 2]),
				matrix);
		}
	}
#endif

	return TRUE;
}

/* material name */
gboolean x3ds_cb_0xA000(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DMaterial *material;
	gchar buffer[1024];

	g_return_val_if_fail(parent->object, FALSE);

	parent->nb -= x3ds_read_cstr(global->f, buffer);
	material = (G3DMaterial *)(parent->object);

	material->name = g_strdup(buffer);

	return TRUE;
}

/* two sided material */
gboolean x3ds_cb_0xA081(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DMaterial *material;

	material = (G3DMaterial *)parent->object;
	g_return_val_if_fail(material, FALSE);

	material->flags |= G3D_FLAG_MAT_TWOSIDE;

	return TRUE;
}

/* texture map name */
gboolean x3ds_cb_0xA300(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DMaterial *material;
	G3DImage *image;
	gchar buffer[512];

	material = (G3DMaterial *)parent->object;
	g_return_val_if_fail(material, FALSE);

	parent->nb -= x3ds_read_cstr(global->f, buffer);

	switch(parent->id)
	{
		case 0xA200: /* texture map */
			material->tex_image = g3d_texture_load_cached(global->context,
				global->model, buffer);
			if(material->tex_image)
			{
				g3d_texture_flip_y(material->tex_image);
				material->tex_image->tex_id = ++ global->max_tex_id;
			}
			break;

		case 0xA210: /* opacity map */
			image = g3d_texture_load(global->context, buffer);
			if(image != NULL)
			{
				g3d_texture_flip_y(image);
				material->tex_image = g3d_texture_merge_alpha(
					material->tex_image, image);
				g3d_texture_free(image);
			}
			break;

		case 0xA220: /* reflection map */
			/* TODO: implement */
			break;

		case 0xA230: /* bump map */
			/* TODO: implement */
			break;

		default:
			g_printerr("[3DS] unhandled texture name in 0x%04X\n", parent->id);
			break;
	}

	return TRUE;
}

/* texture map scale u */
gboolean x3ds_cb_0xA354(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DMaterial *material;
	G3DImage *image;
	gfloat scale;

	material = (G3DMaterial *)parent->object;
	g_return_val_if_fail(material, FALSE);

	image = material->tex_image;
	g_return_val_if_fail(image, FALSE);

	scale = g3d_read_float_le(global->f);
	parent->nb -= 4;

	image->tex_scale_u = scale;
#if DEBUG > 3
	g_print("[3DS] scale_u: %f\n", image->tex_scale_u);
#endif

	return TRUE;
}

/* texture map scale v */
gboolean x3ds_cb_0xA356(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DMaterial *material;
	G3DImage *image;
	gfloat scale;

	material = (G3DMaterial *)parent->object;
	g_return_val_if_fail(material, FALSE);

	image = material->tex_image;
	g_return_val_if_fail(image, FALSE);

	scale = g3d_read_float_le(global->f);
	parent->nb -= 4;

	image->tex_scale_v = scale;
#if DEBUG > 3
	g_print("[3DS] scale_v: %f\n", image->tex_scale_v);
#endif

	return TRUE;
}

/* material */
gboolean x3ds_cb_0xAFFF(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DMaterial *material;
	G3DObject *object;

	material = g3d_material_new();

	if(parent->object)
	{
		object = (G3DObject *)parent->object;
		object->materials = g_slist_append(object->materials, material);
	}
	else
	{
		global->model->materials = g_slist_append(global->model->materials,
			material);
	}

	parent->object = material;

	return TRUE;
}

/* keyframe data header */
gboolean x3ds_cb_0xB00A(x3ds_global_data *global, x3ds_parent_data *parent)
{
	gint32 rev, len;
	gchar buffer[512];

	rev = g3d_read_int16_le(global->f);
	parent->nb -= 2;
	parent->nb -= x3ds_read_cstr(global->f, buffer);
	len = g3d_read_int16_le(global->f);
	parent->nb -= 2;

#if DEBUG > 0
	g_printerr("[3DS] keyframe data: r%d, %d frames, \"%s\"\n",
		rev, len, buffer);
#endif
	return TRUE;
}

/* node header */
gboolean x3ds_cb_0xB010(x3ds_global_data *global, x3ds_parent_data *parent)
{
	GSList *olist;
	G3DObject *object;
	gchar buffer[512];

	parent->nb -= x3ds_read_cstr(global->f, buffer);
#if DEBUG > 3
	g_printerr("[3DS] NODE_HDR: %s\n", buffer);
#endif

	/* find object by name */
	olist = global->model->objects;
	while(olist)
	{
		object = (G3DObject *)olist->data;
		if(strcmp(object->name, buffer) == 0)
		{
			parent->level_object = object;
			break;
		}
		olist = olist->next;
	}

	g3d_read_int16_le(global->f); /* flags 1 */
	g3d_read_int16_le(global->f); /* flags 2 */
	g3d_read_int16_le(global->f); /* ? */
	parent->nb -= 6;

	return TRUE;
}

/* pivot */
gboolean x3ds_cb_0xB013(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DObject *object;
#if 0
	gint32 i;
#endif
	gfloat x, y, z;

	object = parent->level_object;
	if(object == NULL) return FALSE;

	x = g3d_read_float_le(global->f);
	y = g3d_read_float_le(global->f);
	z = g3d_read_float_le(global->f);
	parent->nb -= 12;

#if DEBUG > 3
	g_printerr("[3DS]: PIVOT: (%.2f,%.2f,%.2f)\n", x, y, z);
#endif

	return TRUE;
}

#define X3DS_ENABLE_POS_TRACK_TAG 0

/* position tracking tag */
gboolean x3ds_cb_0xB020(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DObject *object;
	gint32 i, flags, fflags, nkeys, fnum;
#if X3DS_ENABLE_POS_TRACK_TAG
	gint32 j;
#endif
	gfloat x, y, z;

	object = parent->level_object;
	if(object == NULL) return FALSE;

	flags = g3d_read_int16_le(global->f);
	fseek(global->f, 8, SEEK_CUR);
	nkeys = g3d_read_int32_le(global->f);

	parent->nb -= 14;

	for(i = 0; i < nkeys; i ++)
	{
		fnum = g3d_read_int32_le(global->f);
		fflags = g3d_read_int16_le(global->f);
		parent->nb -= 6;

		if(fflags & X3DS_FLAG_TENSION)
		{
			g3d_read_float_le(global->f);
			parent->nb -= 4;
		}
		if(fflags & X3DS_FLAG_CONTINUITY)
		{
			g3d_read_float_le(global->f);
			parent->nb -= 4;
		}
		if(fflags & X3DS_FLAG_BIAS)
		{
			g3d_read_float_le(global->f);
			parent->nb -= 4;
		}
		if(fflags & X3DS_FLAG_EASE_TO)
		{
			g3d_read_float_le(global->f);
			parent->nb -= 4;
		}
		if(fflags & X3DS_FLAG_EASE_FROM)
		{
			g3d_read_float_le(global->f);
			parent->nb -= 4;
		}

		x = g3d_read_float_le(global->f);
		y = g3d_read_float_le(global->f);
		z = g3d_read_float_le(global->f);
		parent->nb -= 12;
#if DEBUG > 2
		g_printerr("[3DS]: POS_TRACK_TAG: frame %d: (%.2f,%.2f,%.2f) (0x%X)\n",
			fnum, x, y, z, fflags);
#endif

#if X3DS_ENABLE_POS_TRACK_TAG
		if(fnum == 0)
		{
			for(j = 0; j < object->vertex_count; j ++)
			{
				object->vertex_data[j * 3 + 0] -= x;
				object->vertex_data[j * 3 + 1] -= y;
				object->vertex_data[j * 3 + 2] -= z;
			}
		}
#endif
	}

	return TRUE;
}

/* rotation tracking tag */
gboolean x3ds_cb_0xB021(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DObject *object;
	gint32 i, j, flags, nkeys, fnum;
	gfloat x, y, z, rot;
	gfloat matrix[16];

	object = parent->level_object;
	if(object == NULL) return FALSE;

	flags = g3d_read_int16_le(global->f);
	fseek(global->f, 8, SEEK_CUR);
	nkeys = g3d_read_int16_le(global->f);
	g3d_read_int16_le(global->f);
	parent->nb -= 14;

	for(i = 0; i < nkeys; i ++)
	{
		fnum = g3d_read_int16_le(global->f);
		g3d_read_int32_le(global->f);
		parent->nb -= 6;

		rot = g3d_read_float_le(global->f);
		x = g3d_read_float_le(global->f);
		y = g3d_read_float_le(global->f);
		z = g3d_read_float_le(global->f);
		parent->nb -= 16;
#if DEBUG > 3
		g_printerr(
			"[3DS]: ROT_TRACK_TAG: frame %d: (%.2f,%.2f,%.2f), %.2f rad\n",
			fnum, x, y, z, rot);
#endif
		if(fnum == -1)
		{
			g3d_matrix_identity(matrix);
			g3d_matrix_rotate(rot, x, y, z, matrix);

			for(j = 0; j < object->vertex_count; j ++)
			{
				g3d_vector_transform(
					&(object->vertex_data[j * 3 + 0]),
					&(object->vertex_data[j * 3 + 1]),
					&(object->vertex_data[j * 3 + 2]),
					matrix);
			}
		}
	}

	return TRUE;

}

/* node id */
gboolean x3ds_cb_0xB030(x3ds_global_data *global, x3ds_parent_data *parent)
{
	gint32 id;

	id = g3d_read_int16_le(global->f);
	parent->nb -= 2;
#if DEBUG > 3
	g_printerr("[3DS] NODE_ID: %d\n", id);
#endif

	return TRUE;
}
