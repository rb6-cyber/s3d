#include <s3d.h>
#include <stdio.h>  /*  NULL */
#include <unistd.h>  /* usleep() */
int i,oid;
void mainloop()
{
	s3d_rotate(oid,0,i,0);
	i++;
	usleep(10000);
}
int main (int argc, char **argv)
{
	if (!s3d_init(&argc,&argv,"3dsloader"))	
	{
		i=argc-1;
		printf("argc = %d\n", argc);
		while (i>0)
		{
		    oid=s3d_import_3ds_file(argv[i]);
		    s3d_flags_on(oid,S3D_OF_VISIBLE);
			i--;
		}
		s3d_mainloop(mainloop);
		s3d_quit();
	}
	return(0);
}
