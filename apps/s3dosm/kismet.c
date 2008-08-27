/*
 * kismet.c
 *
 * Copyright (C) 2006-2008 Simon Wunderlich <dotslash@packetmixer.de>
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
#include <math.h>   /* fabs() */
#include <string.h>   /* strcmp() */
#include <stdlib.h>   /* strtof(),strtod(),strtol() */
#include <libxml/parser.h>
#include <libxml/tree.h>
static int layerid;
static void parse_kismet_node(xmlNodePtr cur)
{
	node_t node;
	xmlAttrPtr attr;
	xmlNodePtr kids, gpskids;

	node_init(&node);

	node.base.layerid = layerid;
	node.base.id = 0;  /* let database decide */
	node.visible = 2; /* something special */
	for (kids = cur->children;kids;kids = kids->next) {
		if (0 == strcmp((char *)kids->name, "gps-info")) {
			for (gpskids = kids->children;gpskids;gpskids = gpskids->next) {
				/* get median value */
				if (0 == strcmp((char *)gpskids->name, "min-lat"))   node.lat = node.lat + strtod((char *)xmlNodeGetContent(gpskids->children), NULL) / 2;
				if (0 == strcmp((char *)gpskids->name, "max-lat"))   node.lat = node.lat + strtod((char *)xmlNodeGetContent(gpskids->children), NULL) / 2;
				if (0 == strcmp((char *)gpskids->name, "min-lon"))   node.lon = node.lon + strtod((char *)xmlNodeGetContent(gpskids->children), NULL) / 2;
				if (0 == strcmp((char *)gpskids->name, "max-lon"))   node.lon = node.lon + strtod((char *)xmlNodeGetContent(gpskids->children), NULL) / 2;
				if (0 == strcmp((char *)gpskids->name, "min-alt"))   node.alt = node.alt + strtod((char *)xmlNodeGetContent(gpskids->children), NULL) / 2;
				if (0 == strcmp((char *)gpskids->name, "max-alt"))   node.alt = node.alt + strtod((char *)xmlNodeGetContent(gpskids->children), NULL) / 2;
			}
		}
	}
	if ((fabs(node.lon) > 0.01) || (fabs(node.lat) > 0.01)) /* really, i don't want to discriminate anyone at 0 lat 0 lon running a wifi hotspot, even
                 if it's in the middle of the ocean. i'm very sorry. */
	{

		db_insert_node(&node);
		for (kids = cur->children;kids;kids = kids->next) {
			if (0 == strcmp((char *)kids->name, "SSID"))    db_add_tag(OBJECT_T(&node), "wifi_SSID", (char *)xmlNodeGetContent(kids->children));
			if (0 == strcmp((char *)kids->name, "BSSID"))    db_add_tag(OBJECT_T(&node), "wifi_BSSID", (char *)xmlNodeGetContent(kids->children));
		}
		for (attr = cur->properties;attr;attr = attr->next) {
			/*  if (0==strcmp((char *)attr->name,"number"))   node->base.id=  strtol((char *)attr->children->content,NULL,10);
			  else */
			if (0 == strcmp((char *)attr->name, "wep"))  db_add_tag(OBJECT_T(&node), "wifi_wep", (char *)attr->children->content);
			else if (0 == strcmp((char *)attr->name, "type"))   db_add_tag(OBJECT_T(&node), "wifi_type", (char *)attr->children->content);
		}

		db_add_tag(OBJECT_T(&node), "amenity", "wifi");
	}
}

/* parse the osm input file */
layer_t *parse_kismet(const char *buf, int length)
{
	xmlDocPtr doc;
	xmlNodePtr cur, c;
	layer_t *layer = layer_new();
	int i = 0;
	float n = 0;


	doc = xmlReadMemory(buf, length, "noname.xml", NULL, 0);
	if (doc == NULL) {
		fprintf(stderr, "Document not parsed successfully.\n");
		return(NULL);
	}
	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) {
		fprintf(stderr, "empty document\n");
		xmlFreeDoc(doc);
		return(NULL);
	}
	layerid = db_insert_layer("kismet");
	for (c = cur->children;  c != NULL;   c = c->next)   n++; /* count */
	for (cur = cur->children;cur != NULL; cur = cur->next) {
		if (cur->type == XML_ELEMENT_NODE) {
			if (0 == strcmp((char *)cur->name, "wireless-network")) {
				parse_kismet_node(cur);
			}
		}
		if ((i++) % 10 == 0) load_update_status(100*((float)i) / n);

	}
	db_flush();
	xmlFreeDoc(doc);
	return(layer);
}
layer_t *load_kismet_file(const char *filename)
{
	int length;
	char *file;
	layer_t *ret;
	if (NULL == (file = read_file(filename, &length))) return(NULL);
	ret = parse_kismet(file, length);
	free(file);
	return ret;
}
