#include <s3d.h>
#include <stdio.h>  /*  NULL */
#include <unistd.h>  /* usleep() */
int i,oid;
void mainloop()
{
	s3d_rotate(oid,0,i,0);
	i++;
	usleep(100000);
}
void object_click(struct s3d_evt *evt)
{
	s3d_quit();
}
	
int main (int argc, char **argv)
{
	if (argc<2)
	{
		printf("usage: %s [somefile.3ds]\n",argv[0]);
		return(-1);
	}
	if (!s3d_init(&argc,&argv,"3dsloader"))	
	{
		s3d_set_callback(S3D_EVENT_OBJ_CLICK,object_click);
	    if (-1!=(oid=s3d_import_3ds_file(argv[1])))
		{
		    s3d_flags_on(oid,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
			s3d_mainloop(mainloop);
		} else {
			printf("file not found ... \n");
		}
		s3d_quit();
	}
	return(0);
}
