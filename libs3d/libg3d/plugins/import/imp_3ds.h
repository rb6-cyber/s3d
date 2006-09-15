#ifndef _IMP_3DS_H
#define _IMP_3DS_H

#include <stdio.h>
#include <glib.h>
#include <g3d/g3d.h>

typedef struct {
    G3DContext *context;
    G3DModel *model;
    FILE *f;
    gfloat scale;
	gint32 max_tex_id;
	long int max_fpos;
} x3ds_global_data;

typedef struct {
    gint32 id;
    gpointer object;
	gpointer misc_object;
    gint32 level;
	gpointer level_object;
    guint32 nb;
} x3ds_parent_data;

typedef gboolean (* x3ds_callback)(x3ds_global_data *global,
    x3ds_parent_data *parent);


gboolean x3ds_read_ctnr(x3ds_global_data *global, x3ds_parent_data *parent);
void x3ds_update_progress(x3ds_global_data *global);
gint32 x3ds_read_cstr(FILE *f, char *string);
G3DObject *x3ds_newobject(G3DModel *model, const char *name);
void x3ds_debug(int level, char *format, ...);

#endif /* _IMP_3DS_H */
