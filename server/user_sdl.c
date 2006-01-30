#include "global.h"
#include <SDL.h>
/*  this file reads user input */
/*  this is done right now by SDL-polling */
int user_init_sdl() {
	return(0);
}
int user_main_sdl() {
  SDL_Event 	event;
  while (SDL_PollEvent(&event)) 
  {
	switch (event.type)
	{
		case SDL_MOUSEMOTION: 
/* 			dprintf(VLOW,"Current mouse position is: (%d, %d),button %d", event.motion.x, event.motion.y,event.button.button); */
			 switch (event.button.button)
			 {
			 	case SDL_BUTTON_LEFT:
			 		user_mouse(0,2,event.motion.x,event.motion.y);break;
			 	case SDL_BUTTON_MIDDLE:
			 		user_mouse(1,2,event.motion.x,event.motion.y);break;
			 	case SDL_BUTTON_RIGHT:
				case SDL_BUTTON_RMASK:
			 		user_mouse(2,2,event.motion.x,event.motion.y);break;
				case 0:break;
					 /*  nno button ... */
				default:
					dprintf(LOW,"don't know button %d", event.button.button);

			 }
			break;
		case SDL_MOUSEBUTTONDOWN:
			 switch (event.button.button)
			 {
			 	case SDL_BUTTON_LEFT:
			 		user_mouse(0,0,event.motion.x,event.motion.y);break;
			 	case SDL_BUTTON_MIDDLE:
			 		user_mouse(1,0,event.motion.x,event.motion.y);break;
			 	case SDL_BUTTON_RIGHT:
			 		user_mouse(2,0,event.motion.x,event.motion.y);break;

			 }
			 break;
		case SDL_MOUSEBUTTONUP:
			 switch (event.button.button)
			 {
			 	case SDL_BUTTON_LEFT:
			 		user_mouse(0,1,event.motion.x,event.motion.y);break;
			 	case SDL_BUTTON_MIDDLE:
			 		user_mouse(1,1,event.motion.x,event.motion.y);break;
			 	case SDL_BUTTON_RIGHT:
			 		user_mouse(2,1,event.motion.x,event.motion.y);break;

			 }
			 break;

		case SDL_KEYDOWN:
			user_key(event.key.keysym.sym,0);
			break;
		case SDL_QUIT:
			dprintf(HIGH,"SDL_QUIT");
			running=0;
			break;
		 /*  these events are not processed right now ... */
		case SDL_ACTIVEEVENT:		dprintf(VLOW,"SDL_ACTIVEEVENT");break;
		case SDL_KEYUP:				dprintf(VLOW,"SDL_KEYUP");break;
		case SDL_SYSWMEVENT:		dprintf(VLOW,"SDL_SYSWMEVENT");break;
		case SDL_VIDEORESIZE:		dprintf(VLOW,"SDL_VIDEORESIZE");break;
		case SDL_VIDEOEXPOSE:		dprintf(VLOW,"SDL_VIDEOEXPOSE");break;
		case SDL_USEREVENT:			dprintf(VLOW,"SDL_USEREVENT");break;
		case SDL_JOYAXISMOTION:		dprintf(VLOW,"SDL_JOYAXISMOTION");break;
		case SDL_JOYBALLMOTION:		dprintf(VLOW,"SDL_JOYBALLMOTION");break;
		case SDL_JOYHATMOTION:		dprintf(VLOW,"SDL_JOYHATMOTION");break;
		case SDL_JOYBUTTONDOWN:		dprintf(VLOW,"SDL_JOYBUTTONDOWN");break;
		case SDL_JOYBUTTONUP:		dprintf(VLOW,"SDL_JOYBUTTONUP");break;
		default:
			dprintf(MED,"SDL_PollEvent(): unhandled event");
		 	break;
	  }
  }
  return(0);

}
int user_quit_sdl() {
	return(0);
}

