#include <time.h>
#include <stdio.h>

struct timespec ctick() {
  struct timespec ticky;
  clock_gettime(CLOCK_MONOTONIC, &ticky);
  return ticky;
}

struct timespec ctock(struct timespec ticky, int niters) {
  struct timespec tocky;
  clock_gettime(CLOCK_MONOTONIC, &tocky);
  unsigned long tv_sec = tocky.tv_sec - ticky.tv_sec;
  unsigned long tv_nsec = tocky.tv_nsec - ticky.tv_nsec;
  fprintf(stderr, "Elapsed: %luns\n", (tv_sec*1000000000 + tv_nsec)/niters);
  return tocky;
}
