#include <glib.h>

#include "imp_maya_obj.h"

gboolean maya_var_set(MayaObject *obj, const gchar *var, gpointer value)
{
	g_hash_table_replace(obj->vars, g_strdup(var), value);

	return TRUE;
}

gpointer maya_var_get(MayaObject *obj, const gchar *var)
{
	gpointer val;

	val = g_hash_table_lookup(obj->vars, var);
	return val;
}

gboolean maya_var_set_double(MayaObject *obj, const gchar *var, gdouble value)
{
	gdouble *pval;

	pval = g_new0(gdouble, 1);
	*pval = value;

	return maya_var_set(obj, var, pval);
}

gdouble maya_var_get_double(MayaObject *obj, const gchar *var, gdouble defval)
{
	gdouble *pval;

	pval = maya_var_get(obj, var);
	if(pval == NULL)
		return defval;

	return *pval;
}
