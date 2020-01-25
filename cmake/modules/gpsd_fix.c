// SPDX-License-Identifier: BSD-3-Clause
/* SPDX-FileCopyrightText: 2007-2015  Sven Eckelmann <sven@narfation.org>
 */

#include <gps.h>

#ifdef __CLASSIC_C__
int main(){
  int ac;
  char*av[];
#else
int main(int ac, char*av[]){
#endif
  struct gps_data_t t;
  sizeof(t.fix);
  return 0;
}
