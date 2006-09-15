#include <string.h>

#include <g3d/model.h>

#include "imp_maya_obj.h"

MayaObject *maya_obj_new(void)
{
	MayaObject *obj;

	obj = g_new0(MayaObject, 1);
	obj->vars = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

	return obj;
}

void maya_obj_free(MayaObject *obj)
{
	g_hash_table_destroy(obj->vars);
	if(obj->name) g_free(obj->name);
	g_free(obj);
}

G3DObject *maya_obj_to_g3d(MayaObject *obj)
{
	G3DObject *object;

	object = g_new0(G3DObject, 1);
	object->name = obj->name ? g_strdup(obj->name) : "(unnamed)";

	return object;
}

static G3DObject *get_by_path(G3DModel *model, gchar *path)
{
	gchar **parts, **partp;
	G3DObject *object = NULL;
	GSList *olist;

	partp = parts = g_strsplit(path, "|", 0);
	olist = model->objects;
	while(*partp)
	{
		while(olist)
		{
			object = (G3DObject *)olist->data;

			if(strcmp(object->name, *partp) == 0) break;

			olist = olist->next;
			object = NULL;
		}

		if(object == NULL) return NULL;

		partp ++;
		olist = object->objects;
	}

	g_strfreev(parts);

	return object;
}

gboolean maya_obj_add_to_tree(MayaObject *obj, G3DModel *model,
	G3DObject *object)
{
	G3DObject *parent = NULL;

	if(obj->parent)
	{
		if(*(obj->parent) == '|')
			parent = get_by_path(model, obj->parent + 1);
		else
			parent = g3d_model_get_object_by_name(model, obj->parent);

		if(parent == NULL)
			g_warning(
				"[Maya] maya_obj_add_to_tree: parent object '%s' not found",
				obj->parent);
	}

	if(parent != NULL)
		parent->objects = g_slist_append(parent->objects, object);
	else
		model->objects = g_slist_append(model->objects, object);

	return TRUE;
}
