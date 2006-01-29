#include <s3d.h>
#include <unistd.h> /* usleep()*/
#include <math.h>	/* sin() */
int a;
int oid_head;
int oid_middle;
int oid_foot;

void mainloop()
{
	float pos;
	a=(a+3)%360;
	pos=sin((a*M_PI)/180)*5;
	if (pos<0) pos*=-1;
	s3d_rotate(oid_head,0,a,0);
	s3d_rotate(oid_middle,0,a,0);
	s3d_rotate(oid_foot,0,a,0);
	s3d_translate(oid_head,		0,3.5+2*pos,0);
	s3d_translate(oid_middle,	0,2+1.25*pos,0);
	s3d_translate(oid_foot,		0,pos,0);
	usleep(10000);
}
int main(int argc, char **argv)
{
	if (!s3d_init(&argc,&argv,"snowman"))
	{
			
		oid_head=s3d_import_3ds_file("objs/snow_head.3ds");
		oid_middle=s3d_import_3ds_file("objs/snow_body.3ds");
		oid_foot=s3d_import_3ds_file("objs/snow_foot.3ds");

/*		s3d_link(oid_foot,oid_head);
 *		s3d_link(oid_middle,oid_head);
 *		s3d_translate(oid_head,0,4,0);
		
 *		s3d_translate(oid_middle,0,-1.5,0); 	* relative to head: *
 *		s3d_translate(oid_foot,0,-3.5,0); */
		
		s3d_scale(oid_middle,1.25);
		s3d_scale(oid_foot,1.5);

		s3d_flags_on(oid_head,S3D_OF_VISIBLE);
		s3d_flags_on(oid_middle,S3D_OF_VISIBLE);
		s3d_flags_on(oid_foot,S3D_OF_VISIBLE);
		s3d_mainloop(mainloop);
		s3d_quit();
	}
	return(0);
}
