/*
 * user.c
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
/*  this file reads user input */
static int ox, oy;
int but = -1;
int user_init(void)
{
	switch (frame_mode) {
#ifdef G_SDL
	case FRAME_SDL:
		user_init_sdl();
		break;
#endif
	default:
		return(-1);
	}
	ox = oy = 0xFFFFFF;
	return(0);
}
int user_main(void)
{
	switch (frame_mode) {
#ifdef G_SDL
	case FRAME_SDL:
		user_main_sdl();
#endif
	default:
		return(0);
	}
}
void user_key(uint16_t key, uint16_t unicode, uint16_t mod, int state)
{
	event_key_pressed(key, unicode, mod, state);
}
void user_mouse(int button, int state, int x, int y)
{
	switch (state) {
	case 0: /*  mouse_down ... */
		switch (button) {
		case 0:
			graphics_pick_obj(x, y);
			break;
		case 1:
			if ((ox != 0xFFFFFF) && (oy != 0xFFFFFF))
				navi_pos(ox - x, oy - y);
			break;
		case 2:
			if ((ox != 0xFFFFFF) && (oy != 0xFFFFFF))
				navi_rot(ox - x, oy - y);
			break;
		case 3:
			navi_fwd();
			break;
		case 4:
			navi_back();
			break;
		default:
			s3dprintf(VLOW, "button is ... %d", button);
		}
		ox = x;
		oy = y;
		event_mbutton_clicked(button, state);
		break;
	case 1:  /*  mouse up */
		ox = oy = 0xFFFFFF;
		event_mbutton_clicked(button, state);
		/*  s3dprintf(LOW,"state is: %d,button is %d",state,button);*/
		break;
	case 2:  /*  mouse still down */
		switch (button) {
		case 1:
			if ((ox != 0xFFFFFF) && (oy != 0xFFFFFF))
				navi_pos(ox - x, oy - y);
			break;
		case 2:
			if ((ox != 0xFFFFFF) && (oy != 0xFFFFFF))
				navi_rot(ox - x, oy - y);
			break;
		case 3:
			navi_fwd();
			break;
		case 4:
			navi_back();
			break;
		default:
			s3dprintf(VLOW, "button is ... %d", button);
		}
		ox = x;
		oy = y;
		break;
	}
	but = button;
	/* mouse changed? */
	ptr_move(x, y);
}
int user_quit(void)
{
	return(0);
}

