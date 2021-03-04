#include <unistd.h>
#include <stdio.h>

int main()
{
   long l1_size = sysconf (_SC_LEVEL1_DCACHE_LINESIZE);
   printf("%d\n", l1_size);
   return 0;
}
