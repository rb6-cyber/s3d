#include "global.h"
/*  this file reads user input */
/*  this is done right now by SDL-polling */
static int ox,oy;
int but=-1;
int user_init() {
	switch (frame_mode)
	{
#ifdef G_GLUT
		case G_GLUT:user_init_glut();break;
#endif
#ifdef G_SDL
    	case G_SDL:user_init_sdl();break;
#endif
		default:return(-1);
	}
	ox=oy=0xFFFFFF;
	return(0);
}
int user_main() {
	switch (frame_mode)
	{
#ifdef G_GLUT
		case G_GLUT:return(0);  /*  glut uses callback functions */
#endif
#ifdef G_SDL
    	case G_SDL:user_main_sdl();
#endif
		default:return(0);
	}
	return(0);
}
void user_key(unsigned short key,int state)
{
	if (state==0)  /*  down */
	{
		dprintf(LOW, "got key %d!!",key);
		switch (key) {
			default:
				event_key_pressed(key);
		}
	}
}
void user_mouse(int button, int state, int x, int y) 
{
	if (state==0)  /*  mouse_down ... */
	{
		switch (button)
		{
			case 0:
				graphics_pick_obj(x,y);	
				break;
			case 1:
				if ((ox!=0xFFFFFF) && (oy!=0xFFFFFF))
					navi_pos(ox-x,oy-y);
				break;
			case 2:
				if ((ox!=0xFFFFFF) && (oy!=0xFFFFFF))
					navi_rot(ox-x,oy-y);
				break;
			case 3:
				navi_fwd();
				break;
			case 4:
				navi_back();
				break;
			default:
				dprintf(VLOW,"button is ... %d", button);
		}
	} 
	 /*  mouse still down */
	if (state==2)
	{
		switch (button)
		{
			case 1:
				if ((ox!=0xFFFFFF) && (oy!=0xFFFFFF))
					navi_pos(ox-x,oy-y);
				break;
			case 2:
				if ((ox!=0xFFFFFF) && (oy!=0xFFFFFF))
					navi_rot(ox-x,oy-y);
				break;
			case 3:
				navi_fwd();
				break;
			case 4:
				navi_back();
				break;
			default:
				dprintf(VLOW,"button is ... %d", button);
		}
	}
	ox=x;
	oy=y;
	but=button;
	if (state==1)  /*  mouse up */
	{
		ox=oy=0xFFFFFF;
/*		dprintf(LOW,"state is: %d,button is %d",state,button);*/
	}
}
int user_quit() {
	return(0);
}

