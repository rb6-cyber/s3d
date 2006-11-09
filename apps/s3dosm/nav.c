#include "s3dosm.h"
#include <s3d.h>
icon_t icons[ICON_NUM]={
		{"objs/accesspoint.3ds",0},
		{"objs/star.3ds",0},
	};
int oidx, oidy;
void nav_loadicons()
{
	int i;
	for (i=0;i<ICON_NUM;i++)
	{
		icons[i].oid=s3d_import_model_file(icons[i].path);
	}
}
void nav_init()
{
	nav_loadicons();
	oidx=s3d_new_object();
	oidy=s3d_new_object();
	s3d_link(oidy,oidx);
}
void nav_center(float la, float lo)
{
	double x[3];
	s3d_rotate(oidy,0,-lo,0);
	s3d_rotate(oidx,-(90-la),0,0);
	calc_earth_to_eukl(lo,la,x);
	s3d_translate(oidx,0,-ESIZE*RESCALE,0);
	s3d_scale(oidx,RESCALE);
}

