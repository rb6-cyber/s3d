/*
 * db.c
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

#include <sqlite3.h>
#include "s3dosm.h"
#include <stdio.h>
#include <string.h> /* stdup() */
#include <unistd.h> /* unlink() */
#include <stdlib.h> /* atoi() */
static char qbuf[QBUF];
static int qlen = 0;
static int tagid = 1;  /* tagid, incremented with each new object */

static sqlite3 *db;
static char *dbFile = NULL;

/* TODO: remove '' for security reasons */
static void clean_string(char *clean, const char *dirty, int n)
{
	strncpy(clean, dirty, n);
	clean[n-1] = 0;
}

int db_add_tag(object_t *obj, const char *key, const char *val)
{
	char tagquery[MAXQ];
	char mkey[MAXQ], mval[MAXQ];
	clean_string(mkey, key, MAXQ);
	clean_string(mval, val, MAXQ);
	sqlite3_snprintf(MAXQ, tagquery, "INSERT INTO tag VALUES (%d, '%q','%q' );", (int)obj->tagid, mkey, mval);
	db_exec(tagquery, NULL, NULL);
	return(0);
}

int db_insert_node(node_t *node)
{
	char addquery[MAXQ];
	node->base.tagid = tagid++;

	if (node->base.id == 0) /* give own id */
		sqlite3_snprintf(MAXQ, addquery, "INSERT INTO node (layer_id, latitude, longitude, altitude, visible, tag_id) VALUES (%d, %f, %f, %f, %d, %d);",
		                 (int)node->base.layerid,    node->lat,  node->lon,  node->alt,  node->visible, (int)node->base.tagid);
	else
		sqlite3_snprintf(MAXQ, addquery, "INSERT INTO node (layer_id, node_id,latitude, longitude, altitude, visible, tag_id) VALUES (%d, %d, %f, %f, %f, %d, %d);",
		                 (int)node->base.layerid, (int)node->base.id, node->lat,  node->lon,  node->alt,  node->visible, (int)node->base.tagid);

	db_exec(addquery, NULL, NULL);
	return(0);
}

int db_insert_segment(segment_t *seg)
{
	char addquery[MAXQ];
	seg->base.tagid = tagid++;


	if (seg->base.id == 0) { /* give own id */
		printf("ugh, segment id is 0!\n");
		exit(0);
	}

	sqlite3_snprintf(MAXQ, addquery, "INSERT INTO segment (layer_id, seg_id, node_from, node_to, tag_id) VALUES (%d, %d, %d, %d, %d);",
	                 (int)seg->base.layerid, (int)seg->base.id, (int)seg->from, (int)seg->to, (int)seg->base.tagid);
	db_exec(addquery, NULL, NULL);

	return(0);
}

int db_insert_way_only(way_t *way)
{
	char addquery[MAXQ];
	way->base.tagid = tagid++;
	sqlite3_snprintf(MAXQ, addquery, "INSERT INTO way (layer_id, way_id, tag_id) VALUES (%d, %d, %d);", (int)way->base.layerid, (int)way->base.id, (int)way->base.tagid);
	db_exec(addquery, NULL, NULL);
	return(0);
}

int db_insert_way_seg(way_t *way, int seg_n)
{
	char addquery[MAXQ];
	sqlite3_snprintf(MAXQ, addquery, "UPDATE segment SET way_id=%d WHERE seg_id=%d AND layer_id=%d;", (int)way->base.id, seg_n, (int)way->base.layerid);
	db_exec(addquery, NULL, NULL);
	return(0);
}

int db_insert_layer(const char *layer_name)
{
	char findquery[MAXQ];
	char addquery[MAXQ];
	char clayer[MAXQ];
	int layerid = -1;
	clean_string(clayer, layer_name, MAXQ);

	sqlite3_snprintf(MAXQ, findquery, "SELECT layer_id FROM layer WHERE name='%q';", clayer);
	db_exec(findquery, db_getint, &layerid);
	if (layerid == -1) { /* need to add */
		sqlite3_snprintf(MAXQ, addquery, "INSERT INTO layer(name) VALUES ('%q');", clayer);
		db_exec(addquery, NULL, NULL);
		db_flush();
		db_exec(findquery, db_getint, &layerid);
	}
	return(layerid);
}

#define MAGIC 1337 /* just to elevate the nodes a little bit */
static int found = 0;
/* tries to find node coordinates of ip, returns 1 if has found something */
int db_olsr_check(const char *ip, float *pos)
{
	char findquery[MAXQ];
	char clean_ip[16];
	float p[6];
	char *s = NULL;
	clean_string(clean_ip, ip, 16);
	if (NULL != (s = strchr(clean_ip, '/')))  /* don't process ip's with subnet information */
		*s = 0; /* TERMINATING ZERO!! */

	sqlite3_snprintf(MAXQ, findquery, "SELECT latitude, longitude, altitude FROM node WHERE tag_id=(SELECT tag_id FROM tag WHERE tagkey='ip' AND tagvalue='%q');", clean_ip);
	found = MAGIC;
	db_exec(findquery, db_getpoint, p);
	if (found == 1) {
		pos[0] = p[0];
		pos[1] = p[1];
		pos[2] = p[2];
		found = 0;
		return(1);
	}
	found = 0;
	return(0);
}

/* initializes the starting point of nodes  by averaging its lon/lat */
int db_olsr_node_init(float *pos)
{
	found = 0;
	db_exec("SELECT AVG(latitude) as latitude, AVG(longitude) as longitude, AVG(altitude) as altitude FROM node WHERE tag_id IN (SELECT tag_id FROM tag WHERE tagkey='ip');", db_getpoint, pos);
	printf("pos = %3.3f %3.3f %3.3f\n", pos[0], pos[1], pos[2]);
	return(0); /* return 1 if something is found, 0 if pos[0] its still 0 */
}


/* expecting a 3x float vector, returns the points coordinates */
int db_getpoint(void *data, int argc, char **argv, char **azColName)
{
	float lo = 0.0, la = 0.0, alt = 0.0;
	float *p = (float*)data;
	int i;
	for (i = 0; i < argc; i++) {
		if (argv[i]) {
			if (0 == strcmp(azColName[i], "longitude"))   lo = strtod(argv[i], NULL);
			else if (0 == strcmp(azColName[i], "latitude"))  la = strtod(argv[i], NULL);
			else if (0 == strcmp(azColName[i], "altitude"))  alt = strtod(argv[i], NULL);
		}
	}
	if (lo == 0.0) {
		printf("missing lo\n");
		exit(0);
	}
	if (la == 0.0) {
		printf("missing la\n");
		exit(0);
	}
	if (found == MAGIC) alt = 2;
	calc_earth_to_eukl(la, lo, alt, p);
	p[3] = la;
	p[4] = lo;
	p[5] = alt;
	found = 1;
	return(0);
}

/* sqlite3-callback to get an integer of the database */
int db_getint(void *tagid, int S3DOSMUNUSED(argc), char **argv, char **S3DOSMUNUSED(azColName))
{
	if (argv[0] != NULL)
		*((int *)tagid) = atoi(argv[0]);
	return 0;
}

/* sqlite3-callback to get a string of the database */
static int db_getstr(void *string, int S3DOSMUNUSED(argc), char **argv, char **S3DOSMUNUSED(azColName))
{
	if (argv[0])
		strncpy((char *)string, argv[0], MAXQ);
	return(0);
}

/* get the value for a a certain tagid and keyvalue (field). Write into target, which has to be allocated with MAXQ bytes of space.
 * Nothing is written when nothing is found. */
int db_gettag(int tagid, const char *field, char *target)
{
	char query[MAXQ];
	target[0] = 0;
	sqlite3_snprintf(MAXQ, query, "SELECT tagvalue FROM tag WHERE tagkey='%q' AND tag_id=%d;", field, tagid);
	db_exec(query, db_getstr, target);
	return(target[0] == 0);
}

int callback(void *S3DOSMUNUSED(NotUsed), int argc, char **argv, char **azColName)
{
	int i;
	for (i = 0; i < argc; i++) {
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

static int db_really_exec(const char *query, sqlite3_callback callback, const void *arg)
{
	char *zErrMsg = NULL;
	int rc;
	if (SQLITE_OK != (rc = sqlite3_exec(db, query, callback, (void*)arg, &zErrMsg))) {
		fprintf(stderr, "query: %s\n", query);
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		exit(-1);
	}
	return(SQLITE_OK != rc); /* 0 = okay */
}

/* call this if you're finished with a few stackable operations */
void db_flush(void)
{
	if (qlen > 0)
		db_really_exec(qbuf, NULL, NULL);
	qbuf[0] = 0;
	qlen = 0;
}

int db_exec(const char *query, sqlite3_callback callback, const void *arg)
{
	int ret;
#ifdef DB_STACK
	if (callback == NULL) { /* we can stack it */
		int len;
		len = strlen(query);
		if (len + qlen >= QBUF)
			db_flush();

		if (len >= QBUF) {
			ret = db_really_exec(query, callback, arg);  /* pass it to the real function */
		} else {
			strncat(qbuf, query, QBUF - qlen - 1);
			qlen += strlen(query);
			ret = 0;
		}
	} else
#endif

	{
		ret = db_really_exec(query, callback, arg);  /* pass it to the real function */
	}
	return(ret);
}

int db_init(const char *dbFile)
{
	int rc;
	tagid = 1;
	qbuf[0] = 0;  /* clear querybuffer */
	qlen = 0;

	unlink(dbFile); /* remove if already there */
	rc = sqlite3_open(dbFile, &db);
	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return(-1);
	}
	return(0);
}

int db_quit(void)
{
	sqlite3_close(db);
	if (dbFile != NULL)
		if (unlink(dbFile))
			perror("db_quit()");
	return(0);
}

int db_create(void)
{
	db_exec("CREATE TABLE node (layer_id INT, node_id INTEGER , latitude DOUBLE PRECISION, longitude DOUBLE PRECISION, altitude DOUBLE PRECISION, visible BOOLEAN, tag_id INT, s3doid INT, PRIMARY KEY(layer_id,node_id));", NULL, NULL);
	db_exec("CREATE TABLE segment (layer_id INT, seg_id INTEGER, node_from INT, node_to INT, tag_id INT, way_id INT,PRIMARY KEY(layer_id,seg_id));", NULL, NULL);
	db_exec("CREATE TABLE way (layer_id INTEGER, way_id INTEGER, tag_id INT, s3doid INT, PRIMARY KEY(layer_id,way_id));", NULL, NULL);
	db_exec("CREATE TABLE layer (layer_id INTEGER, name TEXT, PRIMARY KEY(layer_id));", NULL, NULL);
	db_exec("CREATE TABLE tag (tag_id INT, tagkey TEXT, tagvalue TEXT, PRIMARY KEY(tag_id, tagkey));", NULL, NULL);

	/*
	db_exec("CREATE UNIQUE INDEX node_id_index ON node (node_id,layer_id);", NULL, NULL);
	db_exec("CREATE UNIQUE INDEX segment_id_index ON segment (seg_id,layer_id);", NULL, NULL);
	db_exec("CREATE UNIQUE INDEX tag_id_index ON tag (tag_id,tagkey);", NULL, NULL);
	*/
	db_flush();
	return(0);
}

