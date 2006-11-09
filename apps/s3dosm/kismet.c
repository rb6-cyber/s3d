#include "s3dosm.h"
#include <string.h>			/* strcmp() */
#include <stdlib.h>			/* strtof(),strtod(),strtol() */
#include <libxml/parser.h>
#include <libxml/tree.h>
void add_tag(object_t *obj,char *k, char *v)
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
object_t *parse_kismet_node(xmlNodePtr cur)
{
	node_t *node;
	xmlAttrPtr attr;
	xmlNodePtr kids,gpskids;

	node=node_new();
	attr=cur->properties;
	
	for (attr=cur->properties;attr;attr=attr->next)
	{
		if (0==strcmp((char *)attr->name,"number")) 		node->base.id=		strtol((char *)attr->children->content,NULL,10);
		else if (0==strcmp((char *)attr->name,"wep")) 		add_tag(OBJECT_T(node),"wep",(char *)attr->children->content);
	}
	for (kids=cur->children;kids;kids=kids->next)
	{
		if (0==strcmp((char *)kids->name,"SSID")) 			add_tag(OBJECT_T(node),"SSID",(char *)xmlNodeGetContent(kids->children));
		if (0==strcmp((char *)kids->name,"BSSID")) 			add_tag(OBJECT_T(node),"BSSID",(char *)xmlNodeGetContent(kids->children));
		if (0==strcmp((char *)kids->name,"gps-info"))
		{
			for (gpskids=kids->children;gpskids;gpskids=gpskids->next)
			{
				/* get median value */
				if (0==strcmp((char *)gpskids->name,"min-lat")) 		node->lat=node->lat + strtod((char *)xmlNodeGetContent(gpskids->children),NULL)/2;
				if (0==strcmp((char *)gpskids->name,"max-lat")) 		node->lat=node->lat + strtod((char *)xmlNodeGetContent(gpskids->children),NULL)/2;
				if (0==strcmp((char *)gpskids->name,"min-lon")) 		node->lon=node->lon + strtod((char *)xmlNodeGetContent(gpskids->children),NULL)/2;
				if (0==strcmp((char *)gpskids->name,"max-lon")) 		node->lon=node->lon + strtod((char *)xmlNodeGetContent(gpskids->children),NULL)/2;
				if (0==strcmp((char *)gpskids->name,"min-alt")) 		node->alt=node->alt + strtod((char *)xmlNodeGetContent(gpskids->children),NULL)/2;
				if (0==strcmp((char *)gpskids->name,"max-alt")) 		node->alt=node->alt + strtod((char *)xmlNodeGetContent(gpskids->children),NULL)/2;


			}
		}
	}

	if (node->base.id>0 && (node->lon!=0.0) && (node->lat!=0.0)) /* really, i don't want to discriminate anyone at 0 lat 0 lon running a wifi hotspot, even
																	if it's in the middle of the ocean. i'm very sorry. */
		return(OBJECT_T(node));
	else {
		node_free(node);
		return(NULL);
	}
}

/* parse the osm input file */
layer_t *parse_kismet(char *buf, int length)
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	layer_t *layer=layer_new();
	object_t *obj;
	

	doc = xmlReadMemory(buf, length, "noname.xml", NULL, 0);
	if (doc == NULL) {
		fprintf(stderr,"Document not parsed successfully.\n");
		return(NULL);
	}
	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) {
		fprintf(stderr,"empty document\n");
		xmlFreeDoc(doc);
		return(NULL);
	}
	for (cur=cur->children;cur!=NULL; cur=cur->next)
	{
		if (cur->type==XML_ELEMENT_NODE)
		{
			if (0==strcmp((char *)cur->name,"wireless-network"))
			{
				if (NULL!=(obj=parse_kismet_node(cur)))
					layer->tree=avl_insert(layer->tree, obj);
				else fprintf(stderr,"bad node\n"); 
			} 
		}
	}
	xmlFreeDoc(doc);

	return(layer);
}
