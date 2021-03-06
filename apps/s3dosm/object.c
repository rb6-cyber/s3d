// SPDX-License-Identifier: GPL-2.0-or-later
/* SPDX-FileCopyrightText: 2006-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */


#include "s3dosm.h"
#include <stdlib.h> /* malloc() */

/* ########### object ############### */
void object_free(object_t *obj)
{
	int i;
	for (i = 0; i < obj->tag_n; i++)
		tag_free(&(obj->tag_p[i]));
}
void object_init(object_t *nobj)
{
	nobj->bal = 0;
	nobj->left = NULL;
	nobj->right = NULL;
	nobj->id = 0;
	nobj->oid = -1;
	nobj->layerid = -1;
	nobj->tagid = -1;
	nobj->tag_n = 0;
	nobj->tag_p = NULL;
	nobj->type = T_OBJECT;
}

object_t* object_new(int key)
{
	object_t *nobj = (object_t*)malloc(sizeof(object_t));
	object_init(nobj);
	nobj->id = key;
	return nobj;
}
/* ########### node ############### */
void node_init(node_t *nnode)
{
	object_init((object_t *)nnode);
	OBJECT_T(nnode)->type = T_NODE;
	nnode->lat = 0;
	nnode->lon = 0;
	nnode->alt = 0;
	nnode->visible = 1;
	nnode->vid = -1;
	nnode->adj_n = 0;
	nnode->adj_p = NULL;
}
node_t* node_new(void)
{
	node_t *nnode = (node_t*)malloc(sizeof(node_t));
	node_init(nnode);
	return nnode;
}
void node_free(node_t *node)
{
	free(node);
}

/* ########### segment ############### */
void segment_init(segment_t *nsegment)
{
	object_init((object_t *)nsegment);
	OBJECT_T(nsegment)->type = T_SEGMENT;
	nsegment->from = 0;
	nsegment->to = 0;
}
segment_t* segment_new(void)
{
	segment_t *nsegment = (segment_t*)malloc(sizeof(segment_t));
	segment_init(nsegment);
	return nsegment;
}
void segment_free(segment_t *segment)
{
	free(segment);
}

/* ########### way  ############### */
void way_init(way_t *nway)
{
	object_init((object_t *)nway);
	OBJECT_T(nway)->type = T_WAY;
	nway->seg_n = 0;
	nway->seg_p = NULL;
}
way_t* way_new(void)
{
	way_t *nway = (way_t*)malloc(sizeof(way_t));
	way_init(nway);
	return nway;
}
void way_free(way_t *way)
{
	if (way->seg_n > 0)
		free(way->seg_p);
	free(way);
}
/* ########### layer  ############### */
layer_t* layer_new(void)
{
	layer_t *nlayer = (layer_t*)malloc(sizeof(layer_t));
	nlayer->tree = NULL;
	nlayer->visible = 0;
	return nlayer;
}
/* ########### layerset ############### */
layerset_t layerset = {0, NULL};

void layerset_add(layer_t *layer)
{
	if (layer == NULL) return;
	layerset.n++;
	layerset.p = (layer_t**)realloc(layerset.p, sizeof(layer_t *) * layerset.n);
	layerset.p[layerset.n-1] = layer;
}
