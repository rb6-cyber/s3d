/* SPDX-License-Identifier: LGPL-2.1-or-later */
/* SPDX-FileCopyrightText: 2006-2015  Simon Wunderlich <sw@simonwunderlich.de>
 * SPDX-FileCopyrightText: 1994  A. Narkhede and D .Manocha, who released their code
 * for public domain:
 * <snip>
 *
 * This code is in the public domain. Specifically, we give to the public
 * domain all rights for future licensing of the source code, all resale
 * rights, and all publishing rights.
 *
 * UNC-CH GIVES NO WARRANTY, EXPRESSED OR IMPLIED, FOR THE SOFTWARE
 * AND/OR DOCUMENTATION PROVIDED, INCLUDING, WITHOUT LIMITATION, WARRANTY
 * OF MERCHANTABILITY AND WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 *                                 - Atul Narkhede (narkhede@cs.unc.edu)
 * </snip>
 */


#ifndef __interface_h
#define __interface_h

#define TRUE 1
#define FALSE 0

extern int sei_triangulate_polygon(int, int *, double(*)[2], int (*)[3]);
extern int is_point_inside_polygon(double *);

#endif /* __interface_h */
