/*
 * s3d_keysym.h
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
 * Copyright (C) 1997-2004 Sam Lantinga <slouken@libsdl.org>
 *
 * This file is part of the s3d API, the API of s3d (the 3d network display server).
 * See http://s3d.berlios.de/ for more updates.
 *
 * The s3d API is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * The s3d API is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the s3d API; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* this is just a copy of SDL_keysym.sh from the SDL Simple DirectMedia Layer Package,
 * which seems to have a pretty useful Key definition base.
 * For more Information about SDL, check http://sdlorg*/


/* What we really want is a mapping of every raw key on the keyboard.
   To support international keyboards, we use the range 0xA1 - 0xFF
   as international virtual keycodes.  We'll follow in the footsteps of X11...
   The names of the keys
 */

typedef enum {
	/* The keyboard syms have been cleverly chosen to map to ASCII */
	S3DK_UNKNOWN  = 0,
	S3DK_FIRST  = 0,
	S3DK_BACKSPACE  = 8,
	S3DK_TAB  = 9,
	S3DK_CLEAR  = 12,
	S3DK_RETURN  = 13,
	S3DK_PAUSE  = 19,
	S3DK_ESCAPE  = 27,
	S3DK_SPACE  = 32,
	S3DK_EXCLAIM  = 33,
	S3DK_QUOTEDBL  = 34,
	S3DK_HASH  = 35,
	S3DK_DOLLAR  = 36,
	S3DK_AMPERSAND  = 38,
	S3DK_QUOTE  = 39,
	S3DK_LEFTPAREN  = 40,
	S3DK_RIGHTPAREN  = 41,
	S3DK_ASTERISK  = 42,
	S3DK_PLUS  = 43,
	S3DK_COMMA  = 44,
	S3DK_MINUS  = 45,
	S3DK_PERIOD  = 46,
	S3DK_SLASH  = 47,
	S3DK_0   = 48,
	S3DK_1   = 49,
	S3DK_2   = 50,
	S3DK_3   = 51,
	S3DK_4   = 52,
	S3DK_5   = 53,
	S3DK_6   = 54,
	S3DK_7   = 55,
	S3DK_8   = 56,
	S3DK_9   = 57,
	S3DK_COLON  = 58,
	S3DK_SEMICOLON  = 59,
	S3DK_LESS  = 60,
	S3DK_EQUALS  = 61,
	S3DK_GREATER  = 62,
	S3DK_QUESTION  = 63,
	S3DK_AT   = 64,
	/*
	   Skip uppercase letters
	 */
	S3DK_LEFTBRACKET = 91,
	S3DK_BACKSLASH  = 92,
	S3DK_RIGHTBRACKET = 93,
	S3DK_CARET  = 94,
	S3DK_UNDERSCORE  = 95,
	S3DK_BACKQUOTE  = 96,
	S3DK_a   = 97,
	S3DK_b   = 98,
	S3DK_c   = 99,
	S3DK_d   = 100,
	S3DK_e   = 101,
	S3DK_f   = 102,
	S3DK_g   = 103,
	S3DK_h   = 104,
	S3DK_i   = 105,
	S3DK_j   = 106,
	S3DK_k   = 107,
	S3DK_l   = 108,
	S3DK_m   = 109,
	S3DK_n   = 110,
	S3DK_o   = 111,
	S3DK_p   = 112,
	S3DK_q   = 113,
	S3DK_r   = 114,
	S3DK_s   = 115,
	S3DK_t   = 116,
	S3DK_u   = 117,
	S3DK_v   = 118,
	S3DK_w   = 119,
	S3DK_x   = 120,
	S3DK_y   = 121,
	S3DK_z   = 122,
	S3DK_DELETE  = 127,
	/* End of ASCII mapped keysyms */

	/* International keyboard syms */
	S3DK_WORLD_0  = 160,  /* 0xA0 */
	S3DK_WORLD_1  = 161,
	S3DK_WORLD_2  = 162,
	S3DK_WORLD_3  = 163,
	S3DK_WORLD_4  = 164,
	S3DK_WORLD_5  = 165,
	S3DK_WORLD_6  = 166,
	S3DK_WORLD_7  = 167,
	S3DK_WORLD_8  = 168,
	S3DK_WORLD_9  = 169,
	S3DK_WORLD_10  = 170,
	S3DK_WORLD_11  = 171,
	S3DK_WORLD_12  = 172,
	S3DK_WORLD_13  = 173,
	S3DK_WORLD_14  = 174,
	S3DK_WORLD_15  = 175,
	S3DK_WORLD_16  = 176,
	S3DK_WORLD_17  = 177,
	S3DK_WORLD_18  = 178,
	S3DK_WORLD_19  = 179,
	S3DK_WORLD_20  = 180,
	S3DK_WORLD_21  = 181,
	S3DK_WORLD_22  = 182,
	S3DK_WORLD_23  = 183,
	S3DK_WORLD_24  = 184,
	S3DK_WORLD_25  = 185,
	S3DK_WORLD_26  = 186,
	S3DK_WORLD_27  = 187,
	S3DK_WORLD_28  = 188,
	S3DK_WORLD_29  = 189,
	S3DK_WORLD_30  = 190,
	S3DK_WORLD_31  = 191,
	S3DK_WORLD_32  = 192,
	S3DK_WORLD_33  = 193,
	S3DK_WORLD_34  = 194,
	S3DK_WORLD_35  = 195,
	S3DK_WORLD_36  = 196,
	S3DK_WORLD_37  = 197,
	S3DK_WORLD_38  = 198,
	S3DK_WORLD_39  = 199,
	S3DK_WORLD_40  = 200,
	S3DK_WORLD_41  = 201,
	S3DK_WORLD_42  = 202,
	S3DK_WORLD_43  = 203,
	S3DK_WORLD_44  = 204,
	S3DK_WORLD_45  = 205,
	S3DK_WORLD_46  = 206,
	S3DK_WORLD_47  = 207,
	S3DK_WORLD_48  = 208,
	S3DK_WORLD_49  = 209,
	S3DK_WORLD_50  = 210,
	S3DK_WORLD_51  = 211,
	S3DK_WORLD_52  = 212,
	S3DK_WORLD_53  = 213,
	S3DK_WORLD_54  = 214,
	S3DK_WORLD_55  = 215,
	S3DK_WORLD_56  = 216,
	S3DK_WORLD_57  = 217,
	S3DK_WORLD_58  = 218,
	S3DK_WORLD_59  = 219,
	S3DK_WORLD_60  = 220,
	S3DK_WORLD_61  = 221,
	S3DK_WORLD_62  = 222,
	S3DK_WORLD_63  = 223,
	S3DK_WORLD_64  = 224,
	S3DK_WORLD_65  = 225,
	S3DK_WORLD_66  = 226,
	S3DK_WORLD_67  = 227,
	S3DK_WORLD_68  = 228,
	S3DK_WORLD_69  = 229,
	S3DK_WORLD_70  = 230,
	S3DK_WORLD_71  = 231,
	S3DK_WORLD_72  = 232,
	S3DK_WORLD_73  = 233,
	S3DK_WORLD_74  = 234,
	S3DK_WORLD_75  = 235,
	S3DK_WORLD_76  = 236,
	S3DK_WORLD_77  = 237,
	S3DK_WORLD_78  = 238,
	S3DK_WORLD_79  = 239,
	S3DK_WORLD_80  = 240,
	S3DK_WORLD_81  = 241,
	S3DK_WORLD_82  = 242,
	S3DK_WORLD_83  = 243,
	S3DK_WORLD_84  = 244,
	S3DK_WORLD_85  = 245,
	S3DK_WORLD_86  = 246,
	S3DK_WORLD_87  = 247,
	S3DK_WORLD_88  = 248,
	S3DK_WORLD_89  = 249,
	S3DK_WORLD_90  = 250,
	S3DK_WORLD_91  = 251,
	S3DK_WORLD_92  = 252,
	S3DK_WORLD_93  = 253,
	S3DK_WORLD_94  = 254,
	S3DK_WORLD_95  = 255,  /* 0xFF */

	/* Numeric keypad */
	S3DK_KP0  = 256,
	S3DK_KP1  = 257,
	S3DK_KP2  = 258,
	S3DK_KP3  = 259,
	S3DK_KP4  = 260,
	S3DK_KP5  = 261,
	S3DK_KP6  = 262,
	S3DK_KP7  = 263,
	S3DK_KP8  = 264,
	S3DK_KP9  = 265,
	S3DK_KP_PERIOD  = 266,
	S3DK_KP_DIVIDE  = 267,
	S3DK_KP_MULTIPLY = 268,
	S3DK_KP_MINUS  = 269,
	S3DK_KP_PLUS  = 270,
	S3DK_KP_ENTER  = 271,
	S3DK_KP_EQUALS  = 272,

	/* Arrows + Home/End pad */
	S3DK_UP   = 273,
	S3DK_DOWN  = 274,
	S3DK_RIGHT  = 275,
	S3DK_LEFT  = 276,
	S3DK_INSERT  = 277,
	S3DK_HOME  = 278,
	S3DK_END  = 279,
	S3DK_PAGEUP  = 280,
	S3DK_PAGEDOWN  = 281,

	/* Function keys */
	S3DK_F1   = 282,
	S3DK_F2   = 283,
	S3DK_F3   = 284,
	S3DK_F4   = 285,
	S3DK_F5   = 286,
	S3DK_F6   = 287,
	S3DK_F7   = 288,
	S3DK_F8   = 289,
	S3DK_F9   = 290,
	S3DK_F10  = 291,
	S3DK_F11  = 292,
	S3DK_F12  = 293,
	S3DK_F13  = 294,
	S3DK_F14  = 295,
	S3DK_F15  = 296,

	/* Key state modifier keys */
	S3DK_NUMLOCK  = 300,
	S3DK_CAPSLOCK  = 301,
	S3DK_SCROLLOCK  = 302,
	S3DK_RSHIFT  = 303,
	S3DK_LSHIFT  = 304,
	S3DK_RCTRL  = 305,
	S3DK_LCTRL  = 306,
	S3DK_RALT  = 307,
	S3DK_LALT  = 308,
	S3DK_RMETA  = 309,
	S3DK_LMETA  = 310,
	S3DK_LSUPER  = 311,  /* Left "Windows" key */
	S3DK_RSUPER  = 312,  /* Right "Windows" key */
	S3DK_MODE  = 313,  /* "Alt Gr" key */
	S3DK_COMPOSE  = 314,  /* Multi-key compose key */

	/* Miscellaneous function keys */
	S3DK_HELP  = 315,
	S3DK_PRINT  = 316,
	S3DK_SYSREQ  = 317,
	S3DK_BREAK  = 318,
	S3DK_MENU  = 319,
	S3DK_POWER  = 320,  /* Power Macintosh power key */
	S3DK_EURO  = 321,  /* Some european keyboards */
	S3DK_UNDO  = 322,  /* Atari keyboard has Undo */

	/* Add any other keys here */

	S3DK_LAST
} S3DKey;

/* Enumeration of valid key mods (possibly OR'd together) */
typedef enum {
	S3D_KMOD_NONE  = 0x0000,
	S3D_KMOD_LSHIFT = 0x0001,
	S3D_KMOD_RSHIFT = 0x0002,
	S3D_KMOD_LCTRL = 0x0040,
	S3D_KMOD_RCTRL = 0x0080,
	S3D_KMOD_LALT  = 0x0100,
	S3D_KMOD_RALT  = 0x0200,
	S3D_KMOD_LMETA = 0x0400,
	S3D_KMOD_RMETA = 0x0800,
	S3D_KMOD_NUM   = 0x1000,
	S3D_KMOD_CAPS  = 0x2000,
	S3D_KMOD_MODE  = 0x4000,
	S3D_KMOD_RESERVED = 0x8000
} S3DMod;

#define S3D_KMOD_CTRL (KMOD_LCTRL|KMOD_RCTRL)
#define S3D_KMOD_SHIFT (KMOD_LSHIFT|KMOD_RSHIFT)
#define S3D_KMOD_ALT (KMOD_LALT|KMOD_RALT)
#define S3D_KMOD_META (KMOD_LMETA|KMOD_RMETA)

