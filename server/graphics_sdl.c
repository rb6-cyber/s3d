#include "global.h"

#include <SDL/SDL_opengl.h>
#include <SDL/SDL.h>

int graphics_init_sdl()
{
    SDL_Surface *GLwin = NULL;
    SDL_VideoInfo *VideoInfo;
    int SDLFlags = 0;				 /*  nothing */
    int rgb_size[3]; 				 /*  for SDL_GL attributes */
	dprintf(MED,"Using SDL driver ...");
	
    SDLFlags = SDL_OPENGL;
    SDLFlags |= SDL_GL_DOUBLEBUFFER;
    if (SDL_Init(SDL_INIT_VIDEO) < 0)  		
			errsf("SDL_Init()",SDL_GetError());
    if ((VideoInfo = (SDL_VideoInfo *)SDL_GetVideoInfo())==NULL)
			errs("SDL_GetVIdeoInfo()",SDL_GetError());
    if(VideoInfo -> hw_available) 
	{
		dprintf(LOW,"detected HW_SURFACE");
		SDLFlags |= SDL_HWSURFACE;
	}
	else
	{
		dprintf(LOW,"detected SW_SURFACE");
		SDLFlags |= SDL_SWSURFACE;
	}
    if(VideoInfo -> blit_hw)  	
		SDLFlags |= SDL_HWACCEL;
/*     if(SDL_WM_ToggleFullScreen(GLwin) == 0)        	SDLerror("SDL_WM_ToggleFullScreen"); */


	 /*  set some opengl-attributes */
/*	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, rgb_size[0] );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, rgb_size[1] );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, rgb_size[2] );*/
/* 	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5); */
/* 	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5); */
/* 	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5); */
/* 	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 ); */
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	 /*  more opengl-init-stuff */
	if ((GLwin = SDL_SetVideoMode(X_RES,Y_RES,16,SDLFlags))==NULL) 
			errsf("SDL_SetVideoMode()",SDL_GetError());
	switch (SDL_GetVideoInfo()->vfmt->BitsPerPixel) {
	    case 8:
				rgb_size[0] = 3;	rgb_size[1] = 3;	rgb_size[2] = 2;	break;
	    case 15:
	    case 16:
				rgb_size[0] = 5;	rgb_size[1] = 5;	rgb_size[2] = 5;	break;
            default:
				rgb_size[0] = 8;	rgb_size[1] = 8;	rgb_size[2] = 8;	break;
	}

	 /*  print some information */
	dprintf(VLOW,"Screen BPP: %d", SDL_GetVideoSurface()->format->BitsPerPixel);
	dprintf(VLOW,"Vendor     : %s", glGetString( GL_VENDOR ) );
	dprintf(VLOW,"Renderer   : %s", glGetString( GL_RENDERER ) );
	dprintf(VLOW,"Version    : %s", glGetString( GL_VERSION ) );
	dprintf(VLOW,"Extensions : %s", glGetString( GL_EXTENSIONS ) );
	
	graphics_reshape(X_RES,Y_RES);
	return(0);
}
int graphics_quit_sdl()
{
	SDL_Quit();
	return(0);
}
