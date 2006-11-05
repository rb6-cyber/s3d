#include "s3dosm.h"
#include <stdlib.h> /* malloc() */

/* ########### object ############### */
void object_init(object_t *nobj)
{
	nobj->bal=0;
	nobj->left=NULL;
	nobj->right=NULL;
	nobj->id=0;
	nobj->oid=-1;
	nobj->tag_n=0;
	nobj->tag_p=NULL;
	nobj->type=T_OBJECT;
}

object_t *object_new(int key)
{
	object_t *nobj=malloc(sizeof(object_t));
	object_init(nobj);
	nobj->id=key;
	return (nobj);
}
/* ########### node ############### */
void node_init(node_t *nnode)
{
	object_init((object_t *)nnode);
	OBJECT_T(nnode)->type=T_NODE;
	nnode->lat=0;
	nnode->lon=0;
	nnode->alt=0;
	nnode->visible=1;
	nnode->vid=-1;
	nnode->adj_n=0;
	nnode->adj_p=NULL;
}
node_t *node_new()
{
	node_t *nnode=malloc(sizeof(node_t));
	node_init(nnode);
	return(nnode);
}
void node_free(node_t *node)
{
	free(node);
}

/* ########### segment ############### */
void segment_init(segment_t *nsegment)
{
	object_init((object_t *)nsegment);
	OBJECT_T(nsegment)->type=T_SEGMENT;
	nsegment->from=0;
	nsegment->to=0;
}
segment_t *segment_new()
{
	segment_t *nsegment=malloc(sizeof(segment_t));
	segment_init(nsegment);
	return(nsegment);
}
void segment_free(segment_t *segment)
{
	free(segment);
}

/* ########### way  ############### */
void way_init(way_t *nway)
{
	object_init((object_t *)nway);
	OBJECT_T(nway)->type=T_WAY;
	nway->seg_n=0;
	nway->seg_p=NULL;
}
way_t *way_new()
{
	way_t *nway=malloc(sizeof(way_t));
	way_init(nway);
	return(nway);
}
void way_free(way_t *way)
{
	if (way->seg_n>0) 
		free(way->seg_p);
	free(way);
}
/* ########### layer  ############### */
layer_t *layer_new()
{
	layer_t *nlayer=malloc(sizeof(layer_t));
	nlayer->tree=NULL;
	return(nlayer);
}
