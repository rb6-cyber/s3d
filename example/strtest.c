#include <s3d.h>
#include <stdio.h>  /*  NULL*/
#include <unistd.h> /* sleep() */
int o;
void stop(struct s3d_evt *evt)
{
	s3d_quit();
}

void mainloop()
{
/* 	printf("now it's %s\n",time_str); */
	sleep(1);

}
int main (int argc, char **argv)
{
	if (!s3d_init(&argc,&argv,"clock"))	
	{
		s3d_select_font("vera");
		o=s3d_draw_string("o",NULL);
//		o=s3d_draw_string("T",NULL);
		s3d_flags_on(o,S3D_OF_VISIBLE);

		s3d_set_callback(S3D_EVENT_OBJ_CLICK,	(s3d_cb)stop);
		s3d_set_callback(S3D_EVENT_QUIT,		(s3d_cb)stop);
		s3d_mainloop(mainloop);
		 /*  wait for some object to be clicked */
		s3d_quit();
	}
	return(0);
}


