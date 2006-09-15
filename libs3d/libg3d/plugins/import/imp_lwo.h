#ifndef _IMP_LWO_H
#define _IMP_LWO_H

#include <stdio.h>
#include <g3d/types.h>

#define LWO_FLAG_LWO2          (1 << 0)

typedef struct
{
	gint32 ntags;
	gchar **tags;

	gint32 nclips;
	guint32 *clips;
	gchar **clipfiles;

	gfloat *tex_vertices;

	G3DObject *object;
}
LwoObject;

G3DObject *lwo_create_object(FILE *f, G3DModel *model, guint32 flags);
gint lwo_read_string(FILE *f, char *s);
guint32 lwo_read_vx(FILE *f, guint *index);

#endif /* _IMP_LWO_H */
