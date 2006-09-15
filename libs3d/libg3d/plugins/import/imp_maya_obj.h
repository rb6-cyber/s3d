#ifndef _IMP_MAYA_OBJ_H
#define _IMP_MAYA_OBJ_H

#include <glib.h>
#include <g3d/types.h>

typedef struct {
	gchar *name;
	gchar *parent;
	GHashTable *vars;
	gpointer user_data;
} MayaObject;

MayaObject *maya_obj_new(void);
void maya_obj_free(MayaObject *obj);
G3DObject *maya_obj_to_g3d(MayaObject *obj);
gboolean maya_obj_add_to_tree(MayaObject *obj, G3DModel *model,
	G3DObject *object);

#endif /* _IMP_MAYA_OBJ_H */
