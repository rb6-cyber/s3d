#include "s3d.h"
#include <unistd.h> /* fork(), execl() */
#include <stdio.h> /* printf() */
#include <stdlib.h> /* exit() */
#include <string.h> /* strlen() */
struct menu_entry {
	char *icon, *name, *path;
	int icon_oid, str_oid;
};

struct menu_entry menu[]={
		{"./objs/comp.3ds","filebrowser","./filebrowser",0,0},
		{"./objs/comp.3ds","terminal","s3dvt",0,0},
		{"./objs/comp.3ds","clock","s3dclock",0,0}
};
void object_click(struct s3d_evt *hrmz)
{
	unsigned int i, oid;
	oid=*((unsigned int *)hrmz->buf);
	printf("%d got clicked\n",oid);
	for (i=0;i<(sizeof(menu)/sizeof(struct menu_entry));i++)
	{
		if ((oid==menu[i].icon_oid) || (oid==menu[i].str_oid))
		{
			if (fork()==0)
			{
				execl(menu[i].path,NULL);
				exit(0);
			} 
		}
	}
}
void stop()
{
	s3d_quit();
}

void mainloop()
{
	usleep(100000);
}
int main (int argc, char **argv)
{
	int i;
	if (!s3d_init(&argc,&argv,"app starter"))	
	{
		if (s3d_select_font("vera"))
		{
			printf("font not found\n");
		}
		for (i=0;i<(sizeof(menu)/sizeof(struct menu_entry));i++)
		{
			menu[i].icon_oid=s3d_import_3ds_file(menu[i].icon);
			menu[i].str_oid=s3d_draw_string(menu[i].name,NULL);
			s3d_link(menu[i].str_oid,menu[i].icon_oid);
			s3d_translate(menu[i].icon_oid,0,3*i,0);
			s3d_translate(menu[i].str_oid,2,0,0);
			s3d_flags_on(menu[i].icon_oid,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
			s3d_flags_on(menu[i].str_oid,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
			printf("menu item menu[%d], icon_oid=%d, icon_str=%d\n",i,menu[i].icon_oid,menu[i].str_oid);
		}
		s3d_set_callback(S3D_EVENT_QUIT,stop);
		s3d_set_callback(S3D_EVENT_OBJ_CLICK,object_click);
		s3d_mainloop(mainloop);
		s3d_quit();
	}
	return(0);
}

