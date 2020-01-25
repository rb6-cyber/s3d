/* SPDX-License-Identifier: GPL-2.0-or-later */
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
 * SPDX-FileCopyrightText: 2002  Alexander Graf <helly@gmx.net>
 */

#ifndef _S3DVT_H_
#define _S3DVT_H_

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
} t_char;

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

#endif /* _S3DVT_H_ */
