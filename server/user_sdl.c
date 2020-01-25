// SPDX-License-Identifier: GPL-2.0-or-later
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */

#include "global.h"
#include <SDL.h>
/*  this file reads user input */
/*  this is done right now by SDL-polling */

int user_init_sdl(void)
{
#if SDL_VERSION_ATLEAST(2,0,0)
#else
	SDL_EnableUNICODE(1);
#endif
	return 0;
}

int user_main_sdl(void)
{
	SDL_Event event;
	uint32_t unicode = 0;

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
#if !SDL_VERSION_ATLEAST(2,0,0)
			case SDL_BUTTON_WHEELUP:
				user_mouse(3, 0, event.motion.x, event.motion.y);
				break;
			case SDL_BUTTON_WHEELDOWN:
				user_mouse(4, 0, event.motion.x, event.motion.y);
				break;
#endif
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
#if !SDL_VERSION_ATLEAST(2,0,0)
			case SDL_BUTTON_WHEELUP:
				user_mouse(3, 1, event.motion.x, event.motion.y);
				break;
			case SDL_BUTTON_WHEELDOWN:
				user_mouse(4, 1, event.motion.x, event.motion.y);
				break;
#endif
			default:
				s3dprintf(LOW, "don't know button %d", event.button.button);
			}
			break;

#if SDL_VERSION_ATLEAST(2,0,0)
		case SDL_MOUSEWHEEL:
			if (event.wheel.y > 0) {
				user_mouse(3, 0, event.motion.x, event.motion.y);
				user_mouse(3, 1, event.motion.x, event.motion.y);
			}
			if (event.wheel.y < 0) {
				user_mouse(4, 0, event.motion.x, event.motion.y);
				user_mouse(4, 1, event.motion.x, event.motion.y);
			}
			break;
#endif

		case SDL_KEYDOWN:
		case SDL_KEYUP:
			unicode = 0;
#if SDL_VERSION_ATLEAST(2,0,0)
			if (event.key.keysym.sym < 256) {
				unicode = event.key.keysym.sym;
				if (unicode >= 'a' && unicode <= 'z') {
					int shifted = !!(event.key.keysym.mod & KMOD_SHIFT);
					int capslock = !!(event.key.keysym.mod & KMOD_CAPS);
					if ((shifted ^ capslock) != 0) {
						unicode = SDL_toupper(unicode);
					}
				}
			}
#else
			unicode = event.key.keysym.unicode;
#endif
			user_key(event.key.keysym.sym, unicode, event.key.keysym.mod, (event.type == SDL_KEYUP) ? 1 : 0);
			break;
		case SDL_QUIT:
			s3dprintf(HIGH, "SDL_QUIT");
			quit();
			break;
			/*  these events are not processed right now ... */
#if !SDL_VERSION_ATLEAST(2,0,0)
		case SDL_ACTIVEEVENT:
			s3dprintf(VLOW, "SDL_ACTIVEEVENT");
			break;
#endif
		case SDL_SYSWMEVENT:
			s3dprintf(VLOW, "SDL_SYSWMEVENT");
			break;
#if SDL_VERSION_ATLEAST(2,0,0)
		case SDL_WINDOWEVENT:
			switch (event.window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					graphics_reshape(event.window.data1, event.window.data2);
					break;
			}
			break;
#else
		case SDL_VIDEORESIZE:
			if (SDL_SetVideoMode(event.resize.w, event.resize.h, 16, SDLFlags) == NULL)
				errsf("SDL_SetVideoMode()", SDL_GetError());
			graphics_reshape(event.resize.w, event.resize.h);
			break;
		case SDL_VIDEOEXPOSE:
			s3dprintf(VLOW, "SDL_VIDEOEXPOSE");
			break;
#endif
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
	return 0;

}

int user_quit_sdl(void)
{
	return 0;
}
