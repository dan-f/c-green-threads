#include <stdio.h>
#include "gthr.h"

void say_hi() {
  static int thread_num;
  printf("hello from thread %d\n", ++thread_num);
  gt_yield();
}

int main() {
  gt_init();

  int i;
  for (i = 0; i < 2; i++) {
    int ret;
    if (gt_go(say_hi)) {
      printf("gt_go failed with val: %d\n", ret);
      return 1;
    }
  }

  gt_ret(0);
}
