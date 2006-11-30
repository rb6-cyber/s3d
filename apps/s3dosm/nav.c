#include "s3dosm.h"
#include <s3d.h>
#include <string.h>	/* strcmp() */
#include <stdlib.h>	/* strtod() */
#include <stdio.h>	/* printf() */

icon_t icons[ICON_NUM]={
		{"objs/accesspoint.3ds",0},
		{"objs/star.3ds",0},
	};
int oidx, oidy;
/* load icons, we want to clone each of them later */
void nav_loadicons()
{
	int i;
	for (i=0;i<ICON_NUM;i++)
	{
		icons[i].oid=s3d_import_model_file(icons[i].path);
	}
}
/* load rotation centers */
void nav_init()
{
	nav_loadicons();
	oidx=s3d_new_object();
	oidy=s3d_new_object();
	s3d_link(oidy,oidx);
}
/* center to given latitude longitude */
void nav_center(float la, float lo)
{
	float x[3];
	s3d_rotate(oidy,0,-lo,0);
	s3d_rotate(oidx,-(90-la),0,0);
	calc_earth_to_eukl(lo,la,x);
	s3d_translate(oidx,0,-ESIZE*RESCALE,0);
	s3d_scale(oidx,RESCALE);
}

int get_center(void *data, int argc, char **argv, char **azColName)
{
	float *med=(float *)data;
	int i;
	med[0]=0;
	med[1]=0;
	for(i=0; i<argc; i++){
		if (argv[i]) {
			if (0==strcmp(azColName[i],"la"))			med[0]=strtod(argv[i],NULL);
			else if (0==strcmp(azColName[i],"lo"))		med[1]=strtod(argv[i],NULL);
		}
	}
	return(0);
}
/* find some good center on our own */
void nav_autocenter()
{
	float med[2];
	char query[]="SELECT avg(longitude) as lo, avg(latitude) as la FROM node; ";
	db_exec(query,get_center,med);
	nav_center(med[0],med[1]);
	printf("center to %f,%f\n",med[0],med[1]);
}
