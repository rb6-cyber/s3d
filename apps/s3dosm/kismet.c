/*
 * kismet.c
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
static int layerid;
object_t *parse_kismet_node(xmlNodePtr cur)
{
	node_t *node;
	xmlAttrPtr attr;
	xmlNodePtr kids,gpskids;

	node=node_new();
	attr=cur->properties;

	node->base.layerid=layerid;
	node->base.id=-1;				/* let database decide */
	for (attr=cur->properties;attr;attr=attr->next)
	{
/*		if (0==strcmp((char *)attr->name,"number")) 		node->base.id=		strtol((char *)attr->children->content,NULL,10);
		else */if (0==strcmp((char *)attr->name,"wep")) 		tag_add(OBJECT_T(node),"wifi_wep",(char *)attr->children->content);
		else if (0==strcmp((char *)attr->name,"type")) 		tag_add(OBJECT_T(node),"wifi_type",(char *)attr->children->content);
	}
	for (kids=cur->children;kids;kids=kids->next)
	{
		if (0==strcmp((char *)kids->name,"SSID")) 			tag_add(OBJECT_T(node),"wifi_SSID",(char *)xmlNodeGetContent(kids->children));
		if (0==strcmp((char *)kids->name,"BSSID")) 			tag_add(OBJECT_T(node),"wifi_BSSID",(char *)xmlNodeGetContent(kids->children));
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
	node->visible=2;	/* something special */
	tag_add(OBJECT_T(node), "amenity", "wifi");

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
	layerid=db_insert_layer("kismet");
	printf("kismet layerid is %d\n",layerid);
	for (cur=cur->children;cur!=NULL; cur=cur->next)
	{
		if (cur->type==XML_ELEMENT_NODE)
		{
			if (0==strcmp((char *)cur->name,"wireless-network"))
			{
				if (NULL!=(obj=parse_kismet_node(cur)))
					db_insert_object(obj);
				else fprintf(stderr,"bad node\n"); 
			} 
		}
	}
	db_flush();
	xmlFreeDoc(doc);
	return(layer);
}
layer_t *load_kismet_file(char *filename)
{
	int length;
	char *file;
	layer_t *ret;
	if (NULL==(file=read_file(filename,&length))) return(NULL);
	ret=parse_kismet(file,length);
	free(file);
	return ret;
}
