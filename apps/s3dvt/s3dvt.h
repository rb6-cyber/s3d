/*
 * s3dvt.h
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
 * Copyright (C) 2002 Alexander Graf <helly@gmx.net>
 *
 * This file is part of s3dvt, a 3d terminal emulator for s3d.
 * See http://s3d.berlios.de/ for more updates.
 *
 * s3dvt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * s3dvt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with s3dvt; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <config-s3d.h>

#ifndef S3DVTUNUSED
#if defined(UNUSEDPARAM_ATTRIBUTE)
#define S3DVTUNUSED(x) (x)__attribute__((unused))
#elif defined(UNUSEDPARAM_OMIT)
#define S3DVTUNUSED(x) /* x */
#else
#define S3DVTUNUSED(x) x
#endif
#endif

#define MAX_LINES 50
#define MAX_CHARS 80
#define DEFAULT_FGCOLOR 2
#define DEFAULT_BGCOLOR 0
#define X_RATIO  0.75
#define CS   0.1

#define M_PIPE  1
#define M_PTY  2

/* #define M_LINE  1 */
#define M_CHAR  1

typedef struct char_struct {
	char character;
	char fgcolor;
	char bgcolor;
}t_char;

typedef struct line_struct {
	t_char chars[MAX_CHARS+1];
} t_line;

extern t_line line[MAX_LINES+1];
/* main.c */
void paintit(void);
void term_addchar(char toprint);
/* terminal.c */
void AddChar(char *_toadd);

extern int gotnewdata;
extern int cx, cy;
