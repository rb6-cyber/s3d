#include "s3dosm.h"
#include <stdio.h>	/* printf() */
#include <string.h>	/* strcmp() */
#include <stdlib.h>	/* realloc() */
void tag_add(object_t *obj,char *k, char *v)
{
	tag_t *t;
	obj->tag_n++;
	obj->tag_p=realloc(obj->tag_p,obj->tag_n*sizeof(tag_t));
	if (k!=NULL && v!=NULL)
	{
		t=&(obj->tag_p[obj->tag_n-1]);
		t->ttype=TAG_UNKNOWN;
		t->k=strdup(k);
		t->v=strdup(v);
		t->d.s=v;
		if (0==strcmp(k,"name"))	t->ttype=TAG_NAME;
	}
}
tag_t *tag_get(object_t *obj, char *k)
{
	int i;
	for (i=0;i<obj->tag_n;i++)
	{
		if (0==strcmp(obj->tag_p[i].k,k)) return(&(obj->tag_p[i]));
	}
	return(NULL);
}
