/*
 * user_sdl.c
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of s3d, a 3d network display server.
 * See http://s3d.berlios.de/ for more updates.
 *
 * s3d is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * s3d is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with s3d; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "global.h"
#include <SDL.h>
/*  this file reads user input */
/*  this is done right now by SDL-polling */

int user_init_sdl(void)
{
	SDL_EnableUNICODE(1);
	return(0);
}
int user_main_sdl(void)
{
	SDL_Event  event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_MOUSEMOTION:
			/*    s3dprintf(VLOW,"Current mouse position is: (%d, %d),button %d", event.motion.x, event.motion.y,event.button.button); */
			switch (event.button.button) {
			case SDL_BUTTON_LEFT:
				user_mouse(0, 2, event.motion.x, event.motion.y);
				break;
			case SDL_BUTTON_MIDDLE:
				user_mouse(1, 2, event.motion.x, event.motion.y);
				break;
			case SDL_BUTTON_RIGHT:
			case SDL_BUTTON_RMASK:
				user_mouse(2, 2, event.motion.x, event.motion.y);
				break;
				/*    case SDL_BUTTON_WHEELUP:
				      user_mouse(3,2,event.motion.x,event.motion.y);break;
				    case SDL_BUTTON_WHEELDOWN:
				      user_mouse(4,2,event.motion.x,event.motion.y);break;*/
			case 0:
				user_mouse(-1, -1, event.motion.x, event.motion.y);
				break;
				/*  no button ... */
			default:
				s3dprintf(LOW, "don't know button %d", event.button.button);

			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			switch (event.button.button) {
			case SDL_BUTTON_LEFT:
				user_mouse(0, 0, event.motion.x, event.motion.y);
				break;
			case SDL_BUTTON_MIDDLE:
				user_mouse(1, 0, event.motion.x, event.motion.y);
				break;
			case SDL_BUTTON_RIGHT:
				user_mouse(2, 0, event.motion.x, event.motion.y);
				break;
			case SDL_BUTTON_WHEELUP:
				user_mouse(3, 0, event.motion.x, event.motion.y);
				break;
			case SDL_BUTTON_WHEELDOWN:
				user_mouse(4, 0, event.motion.x, event.motion.y);
				break;
			default:
				s3dprintf(LOW, "don't know button %d", event.button.button);
			}
			break;
		case SDL_MOUSEBUTTONUP:
			switch (event.button.button) {
			case SDL_BUTTON_LEFT:
				user_mouse(0, 1, event.motion.x, event.motion.y);
				break;
			case SDL_BUTTON_MIDDLE:
				user_mouse(1, 1, event.motion.x, event.motion.y);
				break;
			case SDL_BUTTON_RIGHT:
				user_mouse(2, 1, event.motion.x, event.motion.y);
				break;
			case SDL_BUTTON_WHEELUP:
				user_mouse(3, 1, event.motion.x, event.motion.y);
				break;
			case SDL_BUTTON_WHEELDOWN:
				user_mouse(4, 1, event.motion.x, event.motion.y);
				break;
			default:
				s3dprintf(LOW, "don't know button %d", event.button.button);
			}
			break;

		case SDL_KEYDOWN:
			user_key(event.key.keysym.sym, event.key.keysym.unicode, event.key.keysym.mod, 0);
			break;
		case SDL_KEYUP:
			user_key(event.key.keysym.sym, event.key.keysym.unicode, event.key.keysym.mod, 1);
			break;
		case SDL_QUIT:
			s3dprintf(HIGH, "SDL_QUIT");
			quit();
			break;
			/*  these events are not processed right now ... */
		case SDL_ACTIVEEVENT:
			s3dprintf(VLOW, "SDL_ACTIVEEVENT");
			break;
		case SDL_SYSWMEVENT:
			s3dprintf(VLOW, "SDL_SYSWMEVENT");
			break;
		case SDL_VIDEORESIZE:
			if (SDL_SetVideoMode(event.resize.w, event.resize.h, 16, SDLFlags) == NULL)
				errsf("SDL_SetVideoMode()", SDL_GetError());
			graphics_reshape(event.resize.w, event.resize.h);
			break;
		case SDL_VIDEOEXPOSE:
			s3dprintf(VLOW, "SDL_VIDEOEXPOSE");
			break;
		case SDL_USEREVENT:
			s3dprintf(VLOW, "SDL_USEREVENT");
			break;
		case SDL_JOYAXISMOTION:
			s3dprintf(VLOW, "SDL_JOYAXISMOTION");
			break;
		case SDL_JOYBALLMOTION:
			s3dprintf(VLOW, "SDL_JOYBALLMOTION");
			break;
		case SDL_JOYHATMOTION:
			s3dprintf(VLOW, "SDL_JOYHATMOTION");
			break;
		case SDL_JOYBUTTONDOWN:
			s3dprintf(VLOW, "SDL_JOYBUTTONDOWN");
			break;
		case SDL_JOYBUTTONUP:
			s3dprintf(VLOW, "SDL_JOYBUTTONUP");
			break;
		default:
			s3dprintf(MED, "SDL_PollEvent(): unhandled event");
			break;
		}
	}
	return(0);

}
int user_quit_sdl(void)
{
	return(0);
}

