#include <string.h>

#include <g3d/iff.h>
#include <g3d/model.h>
#include <g3d/material.h>
#include <g3d/primitive.h>
#include <g3d/read.h>
#include <g3d/matrix.h>

#include "imp_maya_obj.h"
#include "imp_maya_var.h"

/* compound? */
gboolean maya_cb_CMPD(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	gint32 flags, i;
	gdouble *val;
	gchar *var;
	gchar *padding = "                    ";

	if(local->nb < 26)
	{
		g_warning("[Maya][CMPD] size: %d", local->nb);
		return FALSE;
	}

	/* var */
	var = g_new0(gchar, local->nb - 25);
	fread(var, 1, local->nb - 25, global->f);
	local->nb -= (local->nb - 25);

	/* flags ? */
	flags = g3d_read_int8(global->f);
	local->nb -= 1;

	val = g_new0(gdouble, 3);

	for(i = 0; i < 3; i ++)
	{
		val[i] = g3d_read_double_be(global->f);
		local->nb -= 8;
	}

	g_debug("%s[Maya][CMPD] %s = [%g %g %g]",
		padding + (strlen(padding) - local->level) + 1,
		var,
		val[0], val[1], val[2]);

	g_free(val);
	g_free(var);

	return TRUE;
}

/* creator */
gboolean maya_cb_CREA(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	MayaObject *obj;
	gint32 max_len, flags;
	gchar *buffer, *name;
	gchar *padding = "                    ";

	/* flags ? */
	flags = g3d_read_int8(global->f);
	local->nb -= 1;

	max_len = local->nb;
	buffer = g_malloc(max_len + 1);

	/* object name */
	local->nb -= g3d_read_cstr(global->f, buffer, max_len);
	name = g_strdup(buffer);

	obj = (MayaObject *)local->object;
	if(obj)
		obj->name = g_strdup(buffer);

	/* parent name */
	if(local->nb > 0)
		local->nb -= g3d_read_cstr(global->f, buffer, max_len);
	else
		*buffer = '\0';

	if(obj && *buffer)
		obj->parent = g_strdup(buffer);

	g_debug("%s[Maya][CREA] %s (%s)",
		padding + (strlen(padding) - local->level) + 1,
		name, *buffer ? buffer : "none");

	g_free(buffer);
	g_free(name);

	return TRUE;
}

/* double # */
gboolean maya_cb_DBLn(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	gint32 len, flags, ndbl, i;
	gdouble *val;
	gchar *var;
	gchar *padding = "                         ";

	len = local->nb - 9;
	var = g_malloc(len);
	fread(var, 1, len, global->f);
	local->nb -= len;

	flags = g3d_read_int8(global->f);
	local->nb -= 1;

	ndbl = local->nb / 8;
	val = g_new0(gdouble, ndbl);
	for(i = 0; i < ndbl; i ++)
	{
		val[i] = g3d_read_double_be(global->f);
		local->nb -= 8;
	}

	g_debug("%s[Maya][DBL#] %s (%d doubles) (0x%02X)",
		padding + (strlen(padding) - local->level) + 1,
		var, ndbl, flags);

	if(local->object)
		maya_var_set((MayaObject *)local->object, var, val);
	else
		g_free(val);

	g_free(var);

	return TRUE;
}

/* double 2 */
gboolean maya_cb_DBL2(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	gint32 len, flags;
	gdouble *val;
	gchar *var;
	gchar *padding = "                         ";

	len = local->nb - 17;
	var = g_malloc(len);
	fread(var, 1, len, global->f);
	local->nb -= len;

	flags = g3d_read_int8(global->f);
	local->nb -= 1;

	val = g_new0(gdouble, 2);
	val[0] = g3d_read_double_be(global->f);
	val[1] = g3d_read_double_be(global->f);
	local->nb -= 16;

	g_debug("%s[Maya][DBL2] %s = (%g,%g) (0x%02X)",
		padding + (strlen(padding) - local->level) + 1,
		var, val[0], val[1], flags);

	if(local->object)
		maya_var_set((MayaObject *)local->object, var, val);
	else
		g_free(val);

	g_free(var);

	return TRUE;
}

/* double 3 */
gboolean maya_cb_DBL3(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	gint32 len, flags;
	gdouble *val;
	gchar *var;
	gchar *padding = "                         ";

	len = local->nb - 25;
	var = g_malloc(len);
	fread(var, 1, len, global->f);
	local->nb -= len;

	flags = g3d_read_int8(global->f);
	local->nb -= 1;

	val = g_new0(gdouble, 3);
	val[0] = g3d_read_double_be(global->f);
	val[1] = g3d_read_double_be(global->f);
	val[2] = g3d_read_double_be(global->f);
	local->nb -= 24;

	g_debug("%s[Maya][DBL3] %s = (%g,%g,%g) (0x%02X)",
		padding + (strlen(padding) - local->level) + 1,
		var, val[0], val[1], val[2], flags);

	if(local->object)
		maya_var_set((MayaObject *)local->object, var, val);
	else
		g_free(val);

	g_free(var);

	return TRUE;
}

/* double */
gboolean maya_cb_DBLE(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	gint32 len, flags;
	gdouble val;
	gchar *var;
	gchar *padding = "                         ";

	len = local->nb - 9;
	if(len <= 0)
	{
		g_warning("[Maya][DBLE] length of chunk: %d", local->nb);
		return FALSE;
	}

	var = g_malloc(len);
	fread(var, 1, len, global->f);
	local->nb -= len;

	flags = g3d_read_int8(global->f);
	local->nb -= 1;

	val = g3d_read_double_be(global->f);
	local->nb -= 8;

	g_debug("%s[Maya][DBLE] %s = %g (0x%02X)",
		padding + (strlen(padding) - local->level) + 1,
		var, val, flags);

	if(local->object)
		maya_var_set_double((MayaObject *)local->object, var, val);

	g_free(var);

	return TRUE;
}

/* mesh object */
gboolean maya_cb_DMSH(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	MayaObject *obj;
	G3DObject *object;
	G3DMaterial *material;

	if(local->finalize)
	{
		obj = (MayaObject *)local->object;

		object = (G3DObject *)obj->user_data;
		object->name = obj->name ? g_strdup(obj->name) : "(unnamed mesh)";

		maya_obj_add_to_tree(obj, global->model, object);
		maya_obj_free(obj);
	}
	else
	{
		obj = maya_obj_new();
		local->object = obj;

		material = g3d_material_new();
		material->name = g_strdup("(default material)");

		object = g_new0(G3DObject, 1);
		object->materials = g_slist_append(object->materials, material);

		obj->user_data = object;
	}

	return TRUE;
}

/* float 3 */
gboolean maya_cb_FLT3(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	gint32 len, flags;
	gfloat *val;
	gchar *var;
	gchar *padding = "                         ";

	len = local->nb - 13;
	var = g_malloc(len);
	fread(var, 1, len, global->f);
	local->nb -= len;

	flags = g3d_read_int8(global->f);
	local->nb -= 1;

	val = g_new0(gfloat, 3);
	val[0] = g3d_read_float_be(global->f);
	val[1] = g3d_read_float_be(global->f);
	val[2] = g3d_read_float_be(global->f);
	local->nb -= 12;

	g_debug("%s[Maya][DBL3] %s = (%g; %g; %g) (0x%02X)",
		padding + (strlen(padding) - local->level) + 1,
		var, val[0], val[1], val[2], flags);

	if(local->object)
		maya_var_set((MayaObject *)local->object, var, val);
	else
		g_free(val);

	g_free(var);

	return TRUE;
}

/* mesh */
gboolean maya_cb_MESH(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	G3DObject *object;
	G3DMaterial *material;
	G3DFace *face;
	gint32 x1, x2, x3, x4, i, i1, i2;

	x1 = g3d_read_int16_be(global->f);
	x2 = g3d_read_int16_be(global->f);
	x3 = g3d_read_int16_be(global->f);
	x4 = g3d_read_int16_be(global->f);
	local->nb -= 8;

	object = (G3DObject *)((MayaObject *)local->object)->user_data;
	material = (G3DMaterial *)g_slist_nth_data(object->materials, 0);

	if(x1 == 0x6369)
	{
		object->vertex_count = x4 / 3;
		object->vertex_data = g_new0(gfloat, object->vertex_count * 3);

		for(i = 0; i < object->vertex_count; i ++)
		{
			object->vertex_data[i * 3 + 0] = g3d_read_float_be(global->f);
			object->vertex_data[i * 3 + 1] = g3d_read_float_be(global->f);
			object->vertex_data[i * 3 + 2] = g3d_read_float_be(global->f);
			local->nb -= 12;
		}

		x3 = g3d_read_int16_be(global->f);
		x4 = g3d_read_int16_be(global->f);
		local->nb -= 4;
#if 0
		g_debug("[Maya][MESH] %d %d", x3, x4);
#endif
		i1 = -1;
		i2 = -1;
		for(i = 0; i < x4 / 2; i ++)
		{
			if(i1 == -1)
			{
				i1 = g3d_read_int32_be(global->f) & 0xFFFFFF;
				i2 = g3d_read_int32_be(global->f) & 0xFFFFFF;
				local->nb -= 8;
			}
			else
			{
				face = g_new0(G3DFace, 1);
				face->vertex_count = 4;
				face->vertex_indices = g_new0(guint32, 4);
				face->vertex_indices[0] = i1;
				face->vertex_indices[1] = i2;
				face->vertex_indices[2] =
					g3d_read_int32_be(global->f) & 0xFFFFFF;
				face->vertex_indices[3] =
					g3d_read_int32_be(global->f) & 0xFFFFFF;
				face->material = material;
				local->nb -= 8;
				i1 = face->vertex_indices[3];
				i2 = face->vertex_indices[2];

				object->faces = g_slist_append(object->faces, face);
			}
		}

		x3 = g3d_read_int16_be(global->f);
		x4 = g3d_read_int16_be(global->f);
		local->nb -= 4;
	}
	return TRUE;
}

/* poly cube */
gboolean maya_cb_PCUB(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	MayaObject *obj;
	G3DObject *object;
	G3DMaterial *material;
	gdouble w, h, d;

	if(local->finalize)
	{
		obj = (MayaObject *)local->object;
		g_return_val_if_fail(obj != NULL, FALSE);

		w = maya_var_get_double(obj, "sw", 1.0);
		h = maya_var_get_double(obj, "sh", 1.0);
		d = maya_var_get_double(obj, "sd", 1.0);

		material = g3d_material_new();
		object = g3d_primitive_cube(w, h, d, material);
		object->name = obj->name ? g_strdup(obj->name) : "(unnamed cube)";

		object->materials = g_slist_append(object->materials, material);
		maya_obj_add_to_tree(obj, global->model, object);

		/* destroy object */
		maya_obj_free(obj);
	}
	else
	{
		/* create object */
		obj = maya_obj_new();
		local->object = obj;
	}

	return TRUE;
}

/* string */
gboolean maya_cb_STR_(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	gchar *buffer, *var;
	gchar *padding = "                       ";

	/* variable */
	buffer = g_malloc(local->nb);
	local->nb -= g3d_read_cstr(global->f, buffer, local->nb);
	var = g_strdup(buffer);

	/* value */
	local->nb -= g3d_read_cstr(global->f, buffer, local->nb);

	g_debug("%s[Maya][STR ] %s = '%.*s' (%d characters)",
		padding + (strlen(padding) - local->level) + 1,
		var, 40, buffer, strlen(buffer));

	if(local->object)
		maya_var_set((MayaObject *)local->object, var, g_strdup(buffer));

	g_free(buffer);
	g_free(var);

	return TRUE;
}

/* transformation */
gboolean maya_cb_XFRM(g3d_iff_gdata *global, g3d_iff_ldata *local)
{
	MayaObject *obj;
	G3DObject *object;
	G3DTransformation *tf;
	gdouble *val;

	if(local->finalize)
	{
		obj = (MayaObject *)local->object;

		object = maya_obj_to_g3d(obj);

		tf = g_new0(G3DTransformation, 1);
		g3d_matrix_identity(tf->matrix);

		val = maya_var_get(obj, "t");
		if(val)
			g3d_matrix_translate(val[0], val[1], val[2], tf->matrix);

		val = maya_var_get(obj, "r");
		if(val)
			g3d_matrix_rotate_xyz(val[0], val[1], val[2], tf->matrix);

		val = maya_var_get(obj, "s");
		if(val)
			g3d_matrix_scale(val[0], val[1], val[2], tf->matrix);

		/* g3d_matrix_dump(tf->matrix); */

		object->transformation = tf;

		maya_obj_add_to_tree(obj, global->model, object);

		maya_obj_free(obj);
	}
	else
	{
		obj = maya_obj_new();
		local->object = obj;
	}

	return TRUE;
}
