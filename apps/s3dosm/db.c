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

#include <sqlite3.h>
#include "s3dosm.h"
#include <stdio.h>
#include <string.h> /* stdup() */
#include <unistd.h>	/* unlink() */
#include <stdlib.h>	/* atoi() */
static char qbuf[QBUF];
static int qlen=0;

static int tagid;
static sqlite3 *db;
static char *dbFile=NULL;
int db_exec(const char *query, sqlite3_callback callback, void *arg);
static int db_getint(void *tagid, int argc, char **argv, char **azColName);

/* TODO: remove '' for security reasons */
char *clean_string(char *dirty)
{
	return(strdup(dirty));
}
int db_insert_tags(int tag_n, tag_t *tag_p)
{
	int i;
	char tagquery[MAXQ];
	char *mkey, *mval;
	if (tag_n>0 && tag_p!=NULL)
	{
		 /* add tags */
		tagid++;
		tagquery[0]=0;
		for (i=0;i<tag_n;i++)
		{
			mkey=clean_string(tag_p[i].k);
			mval=clean_string(tag_p[i].v);
			snprintf(tagquery,MAXQ,"INSERT INTO tag VALUES (%d, '%s','%s' );",tagid, mkey, mval);
			db_exec(tagquery, NULL, 0);
			free(mkey);
			free(mval);
		}
		return(tagid);
	} else 
		return(-1);
}
int db_insert_node(node_t *node)
{
	int tagid;
	char addquery[MAXQ];
	tagid= db_insert_tags(node->base.tag_n,node->base.tag_p);
	
	if (node->base.id==-1) /* give own id */
		snprintf(addquery,MAXQ,"INSERT INTO node (layer_id, latitude, longitude, altitude, visible, tag_id) VALUES (%d, %f, %f, %f, %d, %d);",
						(int)node->base.layerid,				node->lat,		node->lon,		node->alt,		node->visible, 		tagid);
	else
		snprintf(addquery,MAXQ,"INSERT INTO node VALUES (%d, %d, %f, %f, %f, %d, %d);",
					(int)node->base.layerid,(int)node->base.id,	node->lat,		node->lon,		node->alt,		node->visible, 		tagid);

	db_exec(addquery, NULL, 0);
	return(0);
}

int db_insert_segment(segment_t *seg)
{
	int tagid;
	char addquery[MAXQ];
	tagid= db_insert_tags(seg->base.tag_n,seg->base.tag_p);

/*	if (seg->base.id==-1) / * give own id * /
		snprintf(addquery,MAXQ,"INSERT INTO segment (layer_id, node_from, node_to, tag_id) VALUES (%d, %d, %d), %d;",
						(int)seg->base.layerid,				(int)seg->from, (int)seg->to,	tagid );
	else*/
		snprintf(addquery,MAXQ,"INSERT INTO segment (layer_id, seg_id, node_from, node_to, tag_id) VALUES (%d, %d, %d, %d, %d);",
						(int)seg->base.layerid,(int)seg->base.id,(int)seg->from, (int)seg->to,	tagid );
	db_exec(addquery, NULL, 0);

	return(0);
}
int db_insert_way(way_t *way)
{
	int tagid,i;
	char addquery[MAXQ];
	tagid= db_insert_tags(way->base.tag_n,way->base.tag_p);
	snprintf(addquery,MAXQ,"INSERT INTO way (layer_id, way_id, tag_id) VALUES (%d, %d, %d);",(int)way->base.layerid, (int)way->base.id, tagid );
	db_exec(addquery, NULL, 0);
	for (i=0;i<way->seg_n;i++) {
		snprintf(addquery,MAXQ,"UPDATE segment SET way_id=%d WHERE seg_id=%d AND layer_id=%d;",(int)way->base.id,(int)way->seg_p[i],(int)way->base.layerid );
		db_exec(addquery, NULL, 0);
	}
	return(0);
}
int db_insert_layer(char *layer_name)
{
	char findquery[MAXQ];
	char addquery[MAXQ];
	char *clayer;
	int layerid=-1;
	clayer=clean_string(layer_name);
	
	snprintf(findquery, MAXQ, "SELECT layer_id FROM layer WHERE name='%s';", clayer);
    db_exec(findquery, db_getint, &layerid);
	if (layerid==-1) /* need to add */
	{
		snprintf(addquery, MAXQ, "INSERT INTO layer(name) VALUES ('%s');", clayer);
    	db_exec(addquery, NULL, 0);db_flush();
    	db_exec(findquery, db_getint, &layerid);
	}
	free(clayer);
	return(layerid);
}
int db_insert_object(object_t *obj)
{
	if (obj==NULL)
	{
		printf("NULL object, run away\n");
		return(-1);
	}
	switch (obj->type) {
		case T_NODE:			return(db_insert_node((node_t *)obj));
		case T_SEGMENT:			return(db_insert_segment((segment_t *)obj));
		case T_WAY:				return(	db_insert_way((way_t *)obj));
		default:break;
	}
	return(-1);
}

static int db_getint(void *tagid, int argc, char **argv, char **azColName){
  if (argv[0]!=NULL) 
	  *((int *)tagid)=atoi(argv[0]);
  return 0;
}
int callback(void *NotUsed, int argc, char **argv, char **azColName){
  int i;
  for(i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}
int static db_really_exec(const char *query, sqlite3_callback callback, void *arg)
{
	char *zErrMsg = 0;
	int rc;
	if(SQLITE_OK !=(  rc = sqlite3_exec(db, query, callback, arg, &zErrMsg))) {
		fprintf(stderr,"query: %s\n",query);
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		exit(-1);
	}
	return(SQLITE_OK!=rc); /* 0 = okay */
}
/* call this if you're finished with a few stackable operations */
void db_flush()
{
	if (qlen>0)
		db_really_exec(qbuf,NULL,0);
	qbuf[0]=0;
	qlen=0;
}
int db_exec(const char *query, sqlite3_callback callback, void *arg)
{
	int ret;
#ifdef DB_STACK
	if (callback==NULL) /* we can stack it */
	{
		int len;
		len=strlen(query);
		if (len+qlen>=QBUF)
			db_flush();
		strncat(qbuf,query,QBUF);
		qlen+=strlen(query);
		ret=0;
	} else 
#endif
	{
		ret=db_really_exec(query,callback,arg);		/* pass it to the real function */
	}
	return(ret);
}
int db_init(char *dbFile)
{
  int rc;
  tagid=1;
  qbuf[0]=0;		/* clear querybuffer */
  qlen=0;

  unlink(dbFile);	/* remove if already there */
  rc = sqlite3_open(dbFile, &db);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return(-1);
  }
  return(0);
}
int db_quit()
{
  sqlite3_close(db);
  if (dbFile!=NULL) 
	if (unlink(dbFile))
		perror("db_quit()");
  return(0);
}
int db_create()
{
	db_exec("CREATE TABLE tag (tag_id INT, tagkey TEXT, tagvalue TEXT);", NULL, 0);
	db_exec("CREATE TABLE layer (layer_id INTEGER PRIMARY KEY, name TEXT, UNIQUE(layer_id));", NULL, 0);
	db_exec("CREATE TABLE node (layer_id INT, node_id INTEGER PRIMARY KEY, latitude DOUBLE PRECISION, longitude DOUBLE PRECISION, altitude DOUBLE PRECISION, visible BOOLEAN, tag_id INT, UNIQUE(layer_id,node_id));",NULL,0);
	db_exec("CREATE TABLE segment (layer_id INT, seg_id INTEGER PRIMARY KEY, node_from INT, node_to INT, tag_id INT, way_id INT,UNIQUE(layer_id,seg_id));", NULL, 0);
	db_exec("CREATE TABLE way (layer_id INTEGER, way_id INTEGER PRIMARY KEY, tag_id INT, UNIQUE(layer_id,way_id));", NULL, 0);
	db_flush();
	return(0);
}

