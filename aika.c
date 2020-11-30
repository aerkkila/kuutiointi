#include <stdio.h>
#include <locale.h>
#include <time.h>
#include <string.h>


void main() {
  setlocale(LC_ALL, "fi_FI.utf8");
  char tmp[100];
  
  time_t aika_t = time(NULL);
  struct tm *aika = localtime(&aika_t);
  strftime(tmp, 100, "%A %d.%m.%Y %H.%M.%S", aika);
  
  puts(tmp);
}
