/*
 * s3dgps.c
 * 
 * Copyright (C) 2004-2006 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of s3d, a 3d network display server.
 * See http://s3d.berlios.de/ for more updates.
 * 
 * s3d is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * s3d is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with s3d; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <s3d.h>
#include <gps.h> 	/* gps_*() */
#include <stdio.h> 	/* printf() */
#include <unistd.h>	/* sleep() */
#include <errno.h>  /* errno */
#include <stdlib.h>	/* malloc(), free() */
#include <string.h> /* strlen() */
#include <math.h>	/* sin(),cos(), M_PI */
#include <simage.h>

#define PIXELFACT	2817.947378
#define PIXELFACTN	281794.7378
#define BFS			1024

#define ICON_ARROW	0

#define ICON_MAX	1

struct map_t {
	int lng, lat,scale;
	char path[BFS];
	struct map_t *next;
};
struct point_t {
	int lng,lat;
	char name[BFS];
	int icon;
	struct point_t *next;
};

struct gps_data_t 	*dgps;
int 				 frame=0;
struct point_t		 posicon;
int					 iconlist[ICON_MAX];
char				*iconnames[]={"objs/arrow.3ds"};

unsigned char 		*recode_image(unsigned char *data, int nc, int w, int h);
int 				 load_mapimage(char *path,float lng, float lat, int scale);
void 				 parse_mapkoords();
void				 load_icons();
void 				 show_gpsdata(struct gps_data_t *dgps);
void				 show_position(struct gps_data_t *dgps);
void				 calc_position(float lng, float lat, float *x, float *y, float *z);

void mainloop()
{
	if ((frame%60)==0)
	{
		printf("polling ...\n");
		if (gps_poll(dgps) < 0) 
		{
			printf("read error on server socket\n");
			s3d_quit();
			return;
		}
		printf("done");
		printf("calling show_gpsdata");
		show_gpsdata(dgps);
	}
	show_position(dgps);
	frame++;

	usleep(1000000/30);
}

/* a bad 2d-calculation right now ... */
void				 calc_position(float lng, float lat, float *x, float *y, float *z)
{
	*x=-(lng-12.934980)*50;
	*y=-3;
	*z=(lat-50.817515)*50;
/*	*x=sinf(M_PI*lng/180.0)*cosf(M_PI*lat/180.0);
	*y=cosf(M_PI*lat/180.0);
	*z=cosf(M_PI*lng/180.0)*cosf(M_PI*lat/180.0);
	printf("position for lng/lat %f %f, xyz: %f %f %f, sin-lng:%f\n",lng,lat,*x,*y,*z,sinf(M_PI*lng/180.0));*/
}
/* recodes the image if it's not in rgba format */
unsigned char *recode_image(unsigned char *data, int nc, int w, int h)
{
	unsigned char *new_block;
	int x,y;
	switch (nc)
	{
		case 1:	new_block=malloc(w*h*4);
			  	for (y=0;y<h;y++)
					for (x=0;x<w;x++)
					{
						 new_block[(y*w+x)*4]=
						 new_block[(y*w+x)*4+1]=
						 new_block[(y*w+x)*4+2]=data[y*w+x];
						 new_block[(y*w+x)*4+3]=255;
					}
				puts("grayscale");
				free(data);
			   	break;
		case 2:	new_block=malloc(w*h*4);
			  	for (y=0;y<h;y++)
					for (x=0;x<w;x++)
					{
						 new_block[(y*w+x)*4]=
						 new_block[(y*w+x)*4+1]=
						 new_block[(y*w+x)*4+2]=data[(y*w+x)*2];
						 new_block[(y*w+x)*4+3]=data[(y*w+x)*2+1];
					}
				puts("grayscale+a");
				free(data);
				break;
		case 3:	new_block=malloc(w*h*4);
			  	for (y=0;y<h;y++)
					for (x=0;x<w;x++)
					{
						 new_block[(y*w+x)*4]=data[(y*w+x)*3];
						 new_block[(y*w+x)*4+1]=data[(y*w+x)*3+1];
						 new_block[(y*w+x)*4+2]=data[(y*w+x)*3+2];
						 new_block[(y*w+x)*4+3]=255;
					}
				puts("rgb");
				free(data);
				break;
		case 4:	new_block=data;
			   	puts("rgba");
				break;
	}
	return(new_block);
}
int 				 load_mapimage(char *path,float lng, float lat, int scale)
{
	int w,h,nc;
	float x,y,z;
	double wr,hr,er;
	unsigned char *data;
	int oid;
	if (simage_check_supported(path))
	{
		printf("file %s can be loaded!!\n",path);
		data=simage_read_image(path,&w,&h,&nc);
		data=recode_image(data,nc,w,h);
		oid=s3d_new_object();
		er=6378.2; /* earth radius */
		hr=(180*asin(((1.024*scale)/(PIXELFACT*2))/er))/M_PI; /* half height of card in degress */
		er=6378.2*cos((lat*180)/M_PI); /* radius at latitude position ... */
		wr=-(180*asin(((1.37*scale)/(PIXELFACT*2))/er))/M_PI; /* half width of card in degress */
		printf("wr = %f, hr = %f slice radius for wr =%f \n",hr,wr,er);
		calc_position(lng-wr,lat-hr,&x,&y,&z);
		y-=scale/10000000.0;
		s3d_push_vertex(oid,x,y,z);
		calc_position(lng+wr,lat-hr,&x,&y,&z);
		y-=scale/10000000.0;
		s3d_push_vertex(oid,x,y,z);
		calc_position(lng+wr,lat+hr,&x,&y,&z);
		y-=scale/10000000.0;
		s3d_push_vertex(oid,x,y,z);
		calc_position(lng-wr,lat+hr,&x,&y,&z);
		y-=scale/10000000.0;
		s3d_push_vertex(oid,x,y,z);
		s3d_push_material_a(oid,
						0.8,	0.0,	0.0	,1.0,
						1.0,	1.0,	1.0	,1.0,
						0.8,	0.0,	0.0	,1.0);
		s3d_push_polygon(oid,0,1,2,0);
		s3d_pep_polygon_tex_coord(oid, 0.0,0.0, 
									   1.0,0.0,
									   1.0,1.0);
		s3d_push_polygon(oid,0,2,3,0);
		s3d_pep_polygon_tex_coord(oid, 0.0,0.0, 
									   1.0,1.0,
									   0.0,1.0);
		s3d_push_texture(oid,w,h);		
		s3d_load_texture(oid,0,0,0,w,h,(char *)data);
		free(data);
		s3d_pep_material_texture(oid,0,0);	 /*  assign texture 0 to material 0 */
	} else {
		printf("can't load %s\n",path);
	}
	return(oid);
}
void load_icons()
{
	int i;
	
	for (i=0;i<ICON_MAX;i++)
		iconlist[i]=s3d_import_3ds_file(iconnames[i]);	
}
void show_gpsdata(struct gps_data_t *dgps)
{
	if (!dgps->online) 
		printf("WARNING: no connection to gps device\n");
	printf("[%d] lat/long: [%f|%f], altitude %f\n",frame,dgps->latitude,dgps->longitude,dgps->altitude);
	printf("speed [kph]: %f",dgps->speed/KNOTS_TO_KPH);
	printf("used %d/%d satellits\n",dgps->satellites_used,dgps->satellites);
	switch (dgps->status)
	{
		case STATUS_NO_FIX:		printf("status: no fix");break;
		case STATUS_FIX:		printf("status: fix");break;
		case STATUS_DGPS_FIX:	printf("status: dgps fix");break;
	}
	switch (dgps->mode)
	{
		case MODE_NOT_SEEN:	printf("mode: not seen yet\n");break;
		case MODE_NO_FIX:	printf("mode: no fix\n");break;
		case MODE_2D:		printf("mode: 2d fix\n");break;
		case MODE_3D:		printf("mode: 3d fix\n");break;
	}
}
int lastfix=0;
void show_position(struct gps_data_t *dgps)
{
	int fix=1;
	float x,y,z,p;
	if (!dgps->online) 
		fix=0;
	switch (dgps->mode)
	{
		case MODE_NOT_SEEN:	fix=0;break;
		case MODE_NO_FIX:	fix=0;break;
	}
	if (fix) {
		calc_position(dgps->longitude,dgps->latitude,&x,&y,&z);
/*		calc_position(dgps->latitude,dgps->longitude,&x,&y,&z);*/
		p=sin(M_PI*((2*frame)%180)/180.0);
/*		if (p<0.0) p*=-1.0;*/
		s3d_translate(posicon.icon,x,y+p,z);
		if (!lastfix)
			s3d_flags_on(posicon.icon,S3D_OF_VISIBLE);
	}
	else 
		if (lastfix)
			s3d_flags_off(posicon.icon,S3D_OF_VISIBLE);
	lastfix=fix;
}
/*
static void update(struct gps_data_t *gpsdata, char *message)
{
	printf("updating!!");
}*/
void parse_mapkoords()
{
	FILE *fp;
	char buf[BFS],*ptr,*sptr,*filename,c;
	char path[BFS];
	char *prefix="/home/dotslash/.s3dgps/";
	int len;
	int word;
	double lng,lat;
	int i;
	long scale;
	
	strncpy(path,prefix,BFS);
	strncpy(path+strlen(path),"map_koord.txt",BFS);
	if (NULL!=(fp=fopen(path,"r")))
	{
		while (!feof(fp))
		{
			ptr=buf;
			/* read one line */
			while ((ptr<(buf+BFS+2)))
			{
				if (!fread(ptr,1,1,fp))
					break;
				c=*ptr;
				if ((c=='\n') || (c=='\0'))
					break;
				ptr++;
			}
			*ptr='\0';
			len=strlen(buf);
			/* now we have one line in the buffer, proably */
			sptr=buf;
			word=0;
			do {
				ptr=sptr;
				/* get word */
				while ((ptr-buf)<len)
				{
					c=*ptr;
					if ((c=='\t') || (c=='\n') || (c==' ') || (c=='\0'))
					break;
					ptr++;
				}
				*(ptr)='\0';
				switch (word)
				{
					case 0: filename=sptr;break;
					case 1: lat=strtod(sptr,NULL);break;
					case 2: lng=strtod(sptr,NULL);break;
					case 3: scale=strtol(sptr,NULL,10);break;
				}
				if (word==3)
				{
					strncpy(path,prefix,BFS);
					strncpy(path+strlen(path),filename,BFS);
					printf("going to load %s (located at %f %f with scale %d)\n",path,lng,lat,(int)scale);
/*					if ((scale>1000) && (scale<100000))*/
					{
						i=load_mapimage(path,lng,lat,scale);
						s3d_flags_on(i,S3D_OF_VISIBLE);
					}
				}
				sptr=(ptr+1); /* move to next word */
				word++;
			} while ((ptr-buf)<len);
		}
		fclose(fp);
	}
}
int main(int argc, char **argv)
{
	char *gpshost;
	char *err_str;
	if (argc>1)
		gpshost=argv[1];
	else 
		gpshost="localhost";
	if (!s3d_init(&argc,&argv,"s3dgps")) 
	{
		printf("connecting to %s\n",gpshost);
		dgps=gps_open(gpshost,"2947");
	    if (dgps==NULL) {
			switch ( errno ) {
				case NL_NOSERVICE: 	err_str = "can't get service entry"; break;
				case NL_NOHOST: 	err_str = "can't get host entry"; break;
				case NL_NOPROTO: 	err_str = "can't get protocol entry"; break;
				case NL_NOSOCK: 	err_str = "can't create socket"; break;
				case NL_NOSOCKOPT: 	err_str = "error SETSOCKOPT SO_REUSEADDR"; break;
				case NL_NOCONNECT: 	err_str = "can't connect to host"; break;
				default:             	err_str = "Unknown"; break;
			}
			printf("no connection to gpsd\n");
			fprintf( stderr, "xgps: no gpsd running or network error: %d, %s\n"	, errno, err_str);
		} else {
			parse_mapkoords();
			load_icons();
			/* init the position */
			posicon.lng=0;
			posicon.lat=0;
			posicon.icon=s3d_clone(iconlist[ICON_ARROW]);
			posicon.next=NULL;
			
			printf("connection established !!\n");
			printf("query ...\n");
			gps_query(dgps, "w+x\n");
			printf("done\n");
			s3d_mainloop(mainloop);
			printf("done\n");
			gps_close(dgps);
		}
		s3d_quit(); 
	}
	printf("program finished\n");
	return(0);
}
