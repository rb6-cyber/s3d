/* SPDX-License-Identifier: GPL-2.0-or-later */
/* SPDX-FileCopyrightText: 2006-2015  Simon Wunderlich <sw@simonwunderlich.de>
 * SPDX-FileCopyrightText: 2006-2015  Marek Lindner <mareklindner@neomailbox.ch>
 */


#ifndef _ALLOCATE_H
#define _ALLOCATE_H 1
#include <stdint.h>



void checkIntegrity(void);
void checkLeak(void);
void *debugMalloc(uint32_t length, int32_t tag);
void *debugRealloc(void *memory, uint32_t length, int32_t tag);
void debugFree(void *memoryParameter, int32_t tag);

#endif
