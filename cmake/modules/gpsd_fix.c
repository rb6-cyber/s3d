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
