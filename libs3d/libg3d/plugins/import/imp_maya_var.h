#ifndef _IMP_MAYA_VAR_H
#define _IMP_MAYA_VAR_H

#include "imp_maya_obj.h"

gboolean maya_var_set(MayaObject *obj, const gchar *var, gpointer value);
gpointer maya_var_get(MayaObject *obj, const gchar *var);

gboolean maya_var_set_double(MayaObject *obj, const gchar *var, gdouble value);
gdouble maya_var_get_double(MayaObject *obj, const gchar *var, gdouble defval);

#endif /* _IMP_MAYA_VAR_H */
