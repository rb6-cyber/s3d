/*
 * osm.c
 * 
 * Copyright (C) 2006 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of s3dosm, a gps card application for s3d.
 * See http://s3d.berlios.de/ for more updates.
 * 
 * s3dosm is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * s3dosm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with s3dosm; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "s3dosm.h"
#include <string.h>			/* strcmp() */
#include <stdlib.h>			/* strtof(),strtod(),strtol() */
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "http_fetcher.h"
static int layerid;
int parse_osm_tags(object_t *obj, xmlNodePtr cur)
{
	xmlNodePtr c;
	xmlAttrPtr attr;
	char *v,*k;
	v=k=NULL;
	for (c=cur->children;c!=NULL; c=c->next)
	{
		if (0==strcmp((char *)c->name,"tag"))	{
			for (attr=c->properties;attr;attr=attr->next)
			{
				if (0==strcmp((char *)attr->name,"k")) 				k=(char *)attr->children->content;
				else if (0==strcmp((char *)attr->name,"v")) 		v=(char *)attr->children->content;
			}
			if (k!=NULL && v!=NULL)
				db_add_tag(obj, k, v);
		}
	}
	return(0);

}
void parse_osm_way(xmlNodePtr cur)
{
	way_t way;
	xmlNodePtr kids;
	xmlAttrPtr attr,kattr;
	int seg;

	way_init(&way);
	
	way.base.layerid=layerid;
	for (attr=cur->properties;attr;attr=attr->next)
		if (0==strcmp((char *)attr->name,"id")) 			way.base.id=	strtol((char *)attr->children->content,NULL,10);
	db_insert_way_only(&way);
	parse_osm_tags(OBJECT_T(&way),cur);
	for (kids=cur->children;kids!=NULL;kids=kids->next)
	{
		if (0==strcmp((char *)kids->name,"seg"))	{
			seg=-1;
			for (kattr=kids->properties;kattr;kattr=kattr->next)
				if (0==strcmp((char *)kattr->name,"id")) 			seg=	strtol((char *)kattr->children->content,NULL,10);
			db_insert_way_seg(&way,seg);
		}
	}
}
void parse_osm_segment(xmlNodePtr cur)
{
	segment_t segment;
	xmlAttrPtr attr;

	segment_init(&segment);
	
	segment.base.layerid=layerid;
	for (attr=cur->properties;attr;attr=attr->next)
	{
		
		if (0==strcmp((char *)attr->name,"id")) 			segment.base.id=	strtol((char *)attr->children->content,NULL,10);
		else if (0==strcmp((char *)attr->name,"from")) 		segment.from=		strtod((char *)attr->children->content,NULL);
		else if (0==strcmp((char *)attr->name,"to")) 		segment.to=			strtod((char *)attr->children->content,NULL);
	}
	if ((segment.base.id>0) && (segment.from>0) && (segment.to>0)) {
		db_insert_segment(&segment);
		parse_osm_tags(OBJECT_T(&segment),cur);
	}
}
void parse_osm_node(xmlNodePtr cur)
{
	node_t node;
	xmlAttrPtr attr;

	node_init(&node);
	attr=cur->properties;
	
	node.base.layerid=layerid;
	for (attr=cur->properties;attr;attr=attr->next)
	{
		if (0==strcmp((char *)attr->name,"id")) 			node.base.id=		strtol((char *)attr->children->content,NULL,10);
		else if (0==strcmp((char *)attr->name,"lat")) 		node.lat=			strtod((char *)attr->children->content,NULL);
		else if (0==strcmp((char *)attr->name,"lon")) 		node.lon=			strtod((char *)attr->children->content,NULL);
		else if (0==strcmp((char *)attr->name,"visible")) 	node.visible=		(0==strcmp((char *)attr->children->content,"true"))?1:0;
		else if (0==strcmp((char *)attr->name,"time")) {}	/* TODO */
	}
	if (node.base.id>0) {
		db_insert_node(&node);
		parse_osm_tags(OBJECT_T(&node),cur);
	}
}
/* parse the osm input file */
layer_t *parse_osm(char *buf, int length)
{
	xmlDocPtr doc;
	xmlNodePtr cur,c;
	layer_t *layer=layer_new();
	object_t *obj;
	float n=0;
	int i=0;
	

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
	layerid=db_insert_layer("osm");
	for (c=cur->children;  c!=NULL;   c=c->next) 		n++; /* count */
	for (cur=cur->children;cur!=NULL; cur=cur->next)
	{
		if (cur->type==XML_ELEMENT_NODE)
		{
			obj=NULL;
			if (0==strcmp((char *)cur->name,"node"))				parse_osm_node(cur);
			else if (0==strcmp((char *)cur->name,"segment"))		parse_osm_segment(cur);
			else if (0==strcmp((char *)cur->name,"way"))			parse_osm_way(cur);
		}
		if ((i++)%200==0) 	load_update_status(100*((float)i)/n); /* report status */
	}
	xmlFreeDoc(doc);
	db_flush();

	return(layer);
}
layer_t *load_osm_web(float minlon, float minlat, float maxlon, float maxlat)
{
	int ret;
	char *user = "foo@packetmixer.de";
	char *pass = "foobar";
	char url[1024];
	char *fileBuf;						/* Pointer to downloaded data */
	layer_t *layer;
	snprintf(url,1024,"www.openstreetmap.org/api/0.3/map?bbox=%f,%f,%f,%f",minlon,minlat,maxlon,maxlat);
	printf("downloading url [ %s ]\n",url);

	http_setAuth(user,pass);
	ret = http_fetch(url, &fileBuf);	/* Downloads page */
	if(ret == -1)
	{	
		http_perror("http_fetch");	
		return(NULL);
	}
	layer=parse_osm(fileBuf, ret);
	/* TODO: cleanup http-lib */
	free(fileBuf);
	return layer;
}
layer_t *load_osm_file(char *filename)
{
	int length;
	char *file;
	layer_t *ret;
	if (NULL==(file=read_file(filename,&length))) return(NULL);
	ret=parse_osm(file,length);
	free(file);
	return ret;
}
