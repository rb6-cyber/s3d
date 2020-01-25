/* SPDX-License-Identifier: GPL-2.0-or-later */
/* examples.h
 *
 * SPDX-FileCopyrightText: 2006-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */

#ifndef _EXAMPLES_H_
#define _EXAMPLES_H_

#include <config-s3d.h>

#ifndef S3DUNUSED
#if defined(UNUSEDPARAM_ATTRIBUTE)
#define S3DUNUSED(x) (x)__attribute__((unused))
#elif defined(UNUSEDPARAM_OMIT)
#define S3DUNUSED(x) /* x */
#else
#define S3DUNUSED(x) x
#endif
#endif

#endif /* _EXAMPLES_H_ */
