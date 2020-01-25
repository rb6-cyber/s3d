/* SPDX-License-Identifier: GPL-2.0-or-later */
/* SPDX-FileCopyrightText: 2006-2015  Marek Lindner <mareklindner@neomailbox.ch>
 * SPDX-FileCopyrightText: 2006-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */

/* this file is taken from the batman project (www.open-mesh.net/batman)
 * to find heap corruptions... */

void checkIntegrity(void);
void checkLeak(void);
void *debugMalloc(unsigned int length, int tag);
void *debugRealloc(void *memory, unsigned int length, int tag);
void debugFree(void *memoryParameter);
