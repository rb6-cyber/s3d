/* SPDX-License-Identifier: GPL-2.0-or-later */
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */

#include <config-s3d.h>

#ifndef DOTMCPUNUSED
#if defined(UNUSEDPARAM_ATTRIBUTE)
#define DOTMCPUNUSED(x) (x)__attribute__((unused))
#elif defined(UNUSEDPARAM_OMIT)
#define DOTMCPUNUSED(x) /* x */
#else
#define DOTMCPUNUSED(x) x
#endif
#endif

int menu_init(void);
void menu_click(int oid);
