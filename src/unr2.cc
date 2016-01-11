// clang -fsanitize=memory -fsanitize-memory-track-origins=2
// -fno-omit-frame-pointer -g -O2 umr2.cc
#include <stdio.h>

int main(int argc, char **argv) {
  int *a = new int[10];
  a[5] = 0;
  volatile int b = a[argc];
  if (b)
    printf("xx\n");
  return 0;
}
