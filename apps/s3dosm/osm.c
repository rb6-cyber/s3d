#include "s3dosm.h"
#include <string.h>			/* strcmp() */
#include <stdlib.h>			/* strtof(),strtod(),strtol() */
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "http_fetcher.h"
int parse_osm_tags(object_t *obj, xmlNodePtr cur)
{
	
	xmlNodePtr c;
	xmlAttrPtr attr;
	char *v,*k;
	tag_t *t;
	int n;
	
	n=0;
	for (c=cur->children;c!=NULL; c=c->next)
	{
		if (0==strcmp((char *)c->name,"tag"))	{
			for (attr=c->properties;attr;attr=attr->next)
			{
				if (0==strcmp((char *)attr->name,"k")) 				k=(char *)attr->children->content;
				else if (0==strcmp((char *)attr->name,"v")) 		v=(char *)attr->children->content;
			}
			if (k==NULL || v==NULL)			printf("bad tag!!\n");
			else 							n++;
		}
	}
	obj->tag_n=n;
	obj->tag_p=malloc(obj->tag_n*sizeof(tag_t));
	n=0;
	for (c=cur->children;c!=NULL; c=c->next)
	{
		if (0==strcmp((char *)c->name,"tag"))	{
			for (attr=c->properties;attr;attr=attr->next)
			{
				if (0==strcmp((char *)attr->name,"k")) 				k=(char *)attr->children->content;
				else if (0==strcmp((char *)attr->name,"v")) 		v=(char *)attr->children->content;
			}
			if (k!=NULL && v!=NULL)
			{
				t=&(obj->tag_p[n]);
				t->ttype=TAG_UNKNOWN;
				t->k=strdup(k);
				t->v=strdup(v);
				t->d.s=v;
				if 		(0==strcmp(k,"name"))	t->ttype=TAG_NAME;

				n++;
			}
		}
	}


	
	return(0);

}
object_t *parse_osm_way(xmlNodePtr cur)
{
	way_t *way;
	xmlNodePtr kids;
	xmlAttrPtr attr,kattr;
	int n=0;

	way=way_new();
	
	for (attr=cur->properties;attr;attr=attr->next)
		if (0==strcmp((char *)attr->name,"id")) 			way->base.id=	strtol((char *)attr->children->content,NULL,10);
	/* count segments */
	for (kids=cur->children;kids!=NULL;kids=kids->next)
	{
		if (0==strcmp((char *)kids->name,"seg"))			n++;
	}
	/* add segments in segment buffer */
	if (n>0)
	{
		way->seg_n=n;
		way->seg_p=malloc(sizeof(ID_T)*n);
		n=0;
		for (kids=cur->children;kids!=NULL;kids=kids->next)
		{
			if (0==strcmp((char *)kids->name,"seg"))	{
				for (kattr=kids->properties;kattr;kattr=kattr->next)
					if (0==strcmp((char *)kattr->name,"id")) 			way->seg_p[n]=	strtol((char *)kattr->children->content,NULL,10);
				n++;
			}
		}
	}

	parse_osm_tags(OBJECT_T(way),cur);
	if (way->base.id>0)
		return(OBJECT_T(way));
	else {
		way_free(way);
		return(NULL);
	}
}
object_t *parse_osm_segment(xmlNodePtr cur)
{
	segment_t *segment;
	xmlAttrPtr attr;

	segment=segment_new();
	
	for (attr=cur->properties;attr;attr=attr->next)
	{
		
		if (0==strcmp((char *)attr->name,"id")) 			segment->base.id=	strtol((char *)attr->children->content,NULL,10);
		else if (0==strcmp((char *)attr->name,"from")) 		segment->from=		strtod((char *)attr->children->content,NULL);
		else if (0==strcmp((char *)attr->name,"to")) 		segment->to=		strtod((char *)attr->children->content,NULL);
	}
	parse_osm_tags(OBJECT_T(segment),cur);
	if ((segment->base.id>0) && (segment->from>0) && (segment->to>0))
		return(OBJECT_T(segment));
	else {
		segment_free(segment);
		return(NULL);
	}
}
object_t *parse_osm_node(xmlNodePtr cur)
{
	node_t *node;
	xmlAttrPtr attr;

	node=node_new();
	attr=cur->properties;
	
	for (attr=cur->properties;attr;attr=attr->next)
	{
		if (0==strcmp((char *)attr->name,"id")) 			node->base.id=		strtol((char *)attr->children->content,NULL,10);
		else if (0==strcmp((char *)attr->name,"lat")) 		node->lat=			strtod((char *)attr->children->content,NULL);
		else if (0==strcmp((char *)attr->name,"lon")) 		node->lon=			strtod((char *)attr->children->content,NULL);
		else if (0==strcmp((char *)attr->name,"visible")) 	node->visible=		(0==strcmp((char *)attr->children->content,"true"))?1:0;
		else if (0==strcmp((char *)attr->name,"time")) {}	/* TODO */
	}
	parse_osm_tags(OBJECT_T(node),cur);
	if (node->base.id>0)
		return(OBJECT_T(node));
	else {
		node_free(node);
		return(NULL);
	}
}

void debug_obj(object_t *obj, void *dummy)
{
	int i;
	way_t *way=WAY_T(obj);
	node_t *node=NODE_T(obj);
	segment_t *seg=SEGMENT_T(obj);
	switch (obj->type)
	{
		case T_OBJECT:
				printf("object %d\n",(int)obj->id);
				break;
		case T_NODE:
				 printf("node %d [%f,%f,%f]\n",(int)obj->id,node->lon,node->lat,node->alt);
				 break;
		case T_SEGMENT:
				 printf("segment %d [%d->%d]\n",(int)obj->id,(int)seg->from,(int)seg->to);
				 break;
		case T_WAY:
				 printf("way %d [ ",(int)obj->id);
				 for (i=0;i<way->seg_n;i++)
					printf("%d ",(int)way->seg_p[i]);
				 printf("]\n");
				 break;
				 
	}
	for (i=0;i<obj->tag_n;i++)
		printf("tag %d: %s -> %s\n",i,obj->tag_p[i].k,obj->tag_p[i].v);
}
/* parse the osm input file */
layer_t *parse_osm(char *buf, int length)
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
			if (0==strcmp((char *)cur->name,"node"))
			{
				if (NULL!=(obj=parse_osm_node(cur)))
					layer->tree=avl_insert(layer->tree, obj);
				else fprintf(stderr,"bad node\n"); 
			} else if (0==strcmp((char *)cur->name,"segment"))
			{
				if (NULL!=(obj=parse_osm_segment(cur)))
					layer->tree=avl_insert(layer->tree, obj);
				else fprintf(stderr,"bad segment\n");
			} else if (0==strcmp((char *)cur->name,"way"))
			{
				if (NULL!=(obj=parse_osm_way(cur)))
					layer->tree=avl_insert(layer->tree, obj);
				else fprintf(stderr,"bad way\n");
			}
		}
	}
	xmlFreeDoc(doc);

	return(layer);
}
layer_t *load_osm_web(float minlon, float minlat, float maxlon, float maxlat)
{
	int ret;
	char *user = "foo@packetmixer.de";
	char *pass = "foobar";
	char url[1024];
	char *fileBuf;						/* Pointer to downloaded data */
	snprintf(url,1024,"www.openstreetmap.org/api/0.3/map?bbox=%f,%f,%f,%f",minlon,minlat,maxlon,maxlat);
	printf("downloading url [ %s ]\n",url);

	http_setAuth(user,pass);
	ret = http_fetch(url, &fileBuf);	/* Downloads page */
	if(ret == -1)
	{	
		http_perror("http_fetch");	
		return(NULL);
	}
	return parse_osm(fileBuf, ret);
}
layer_t *load_osm_file(char *filename)
{
	int length;
	char *file;
	if (NULL==(file=read_file(filename,&length))) return(NULL);
	return parse_osm(file,length);
}
