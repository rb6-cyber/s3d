#include "global.h"
#include <math.h>
int alphax,alphay;
float view_x,view_y,view_z;
void navi_right()
{
	navi_pos(1,0);
}
void navi_left()
{
	navi_pos(-1,0);
}
void navi_fwd()
{
	navi_pos(0,1);
}
void navi_back()
{
	navi_pos(0,-1);
}
/* simple movements, not needed currently 
void navi_rot_right()
{
	cam.rotate.y=cam.rotate.y+2;
	if (cam.rotate.y>360) cam.rotate.y-=360;
}
void navi_rot_left()
{
	cam.rotate.y=cam.rotate.y-2;
	cam.rotate.y=(cam.rotate.y<0)?cam.rotate.y+360:cam.rotate.y;
}
void navi_rot_up()
{
	cam.rotate.x=(cam.rotate.x+2);
	if (cam.rotate.x>90) cam.rotate.x=90;
}
void navi_rot_down()
{
	cam.rotate.x=cam.rotate.x-2;
	if (cam.rotate.x<-90) cam.rotate.x=-90;
}*/
void navi_pos(int xdif, int ydif)
{
	float tv[3];
	struct t_obj *cam;
	cam=get_proc_by_pid(MCP)->object[0];
	tv[0]=cam->translate.x;
	tv[1]=cam->translate.y;
	tv[2]=cam->translate.z;

	tv[0]+=ydif*sin((-cam->rotate.y*M_PI)/180);
	tv[2]-=ydif*cos((-cam->rotate.y*M_PI)/180);

	tv[0]-=xdif*cos((-cam->rotate.y*M_PI)/180);
	tv[2]-=xdif*sin((-cam->rotate.y*M_PI)/180);
	obj_translate(get_proc_by_pid(MCP),0,tv);
}
void navi_rot(int xdif, int ydif)
{
	float rv[3];
	struct t_obj *cam;
	cam=get_proc_by_pid(MCP)->object[0];
	rv[0]=(cam->rotate.x+ydif);
	rv[1]=(cam->rotate.y+xdif);
	rv[2]=0.0F;
	if (rv[0]>90) 	rv[0]=90;
	if (rv[0]<-90) 	rv[0]=-90;
	if (rv[1]>360) 	rv[1]-=360;
	if (rv[1]<0) 	rv[1]+=360;
	obj_rotate(get_proc_by_pid(MCP),0,rv);
}
