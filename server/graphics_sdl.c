// SPDX-License-Identifier: GPL-2.0-or-later
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */

#include "global.h"
int aa_level = 4;

#include <SDL_opengl.h>
#include <SDL.h>

int SDLFlags = 0;		/*  some flags for SDL */

#if SDL_VERSION_ATLEAST(2,0,0)
SDL_Window *sdl_window;
#endif

int graphics_init_sdl(void)
{
#if SDL_VERSION_ATLEAST(2,0,0)
	SDL_GLContext glcontext;
#else
	SDL_Surface *GLwin = NULL;
	SDL_VideoInfo *VideoInfo;
#endif
	int buffers, samples;
	s3dprintf(MED, "Using SDL driver ...");

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
		errsf("SDL_Init()", SDL_GetError());
#if SDL_VERSION_ATLEAST(2,0,0)
	SDLFlags = SDL_WINDOW_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_WINDOW_RESIZABLE;

	sdl_window = NULL;
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	do {
		if (sdl_window)
			SDL_DestroyWindow(sdl_window);

		if (aa_level > 0) {
			if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1))
				s3dprintf(VHIGH, "error initializing multisampling");
			if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, aa_level))
				s3dprintf(VHIGH, "no multisampling available");
		}

		sdl_window = SDL_CreateWindow("S3D", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, X_RES, Y_RES, SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
		if (!sdl_window)
			errs("SDL_CreateWindow()", SDL_GetError());

		/*  more opengl-init-stuff */
		glcontext = SDL_GL_CreateContext(sdl_window);
		if (!glcontext) {
			if (aa_level > 0) {
				s3dprintf(MED, "retry without multisampling");
				aa_level = 0;
				SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
				SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
			} else {
				errsf("SDL_GL_CreateContext()", SDL_GetError());
			}
		}
	} while (!glcontext);

#else
	SDLFlags = SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_HWPALETTE | SDL_RESIZABLE;
	if ((VideoInfo = (SDL_VideoInfo *) SDL_GetVideoInfo()) == NULL)
		errs("SDL_GetVIdeoInfo()", SDL_GetError());
	else {
		if (VideoInfo->hw_available) {
			s3dprintf(LOW, "detected HW_SURFACE");
			SDLFlags |= SDL_HWSURFACE;
		} else {
			s3dprintf(LOW, "detected SW_SURFACE");
			SDLFlags |= SDL_SWSURFACE;
		}
		if (VideoInfo->blit_hw)
			SDLFlags |= SDL_HWACCEL;
	}

	/*  set some opengl-attributes */
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	do {
		if (aa_level > 0) {
			if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1))
				s3dprintf(VHIGH, "error initializing multisampling");
			if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, aa_level))
				s3dprintf(VHIGH, "no multisampling available");
		}

		/*  more opengl-init-stuff */
		if ((GLwin = SDL_SetVideoMode(X_RES, Y_RES, 16, SDLFlags)) == NULL) {
			if (aa_level > 0) {
				s3dprintf(MED, "retry without multisampling");
				aa_level = 0;
				SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
				SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
			} else
				errsf("SDL_SetVideoMode()", SDL_GetError());
		}
	} while (GLwin == NULL);
	SDL_WM_SetCaption("S3D", "S3D");
#endif

	if (aa_level > 0) {
		SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &buffers);
		SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &samples);
		s3dprintf(LOW, "Buffers: %d Samples: %d", buffers, samples);
	}

	/*  print some information */
#if !SDL_VERSION_ATLEAST(2,0,0)
	s3dprintf(MED, "Screen BPP: %d", SDL_GetVideoSurface()->format->BitsPerPixel);
#endif
	s3dprintf(VLOW, "Vendor     : %s", glGetString(GL_VENDOR));
	s3dprintf(VLOW, "Renderer   : %s", glGetString(GL_RENDERER));
	s3dprintf(VLOW, "Version    : %s", glGetString(GL_VERSION));
	s3dprintf(VLOW, "Extensions : %s", glGetString(GL_EXTENSIONS));

	graphics_reshape(X_RES, Y_RES);
	return 0;
}

int graphics_quit_sdl(void)
{
	SDL_Quit();
	return 0;
}
