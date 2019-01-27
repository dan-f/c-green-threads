#ifndef GREEN_THREADS
#define GREEN_THREADS

#include <stdbool.h>
#include <stdint.h>

/* The thread's non-volatile/callee-saved registers; must be preserved across
   calls.  Hence  */
struct gt_ctx {
  uint64_t rsp;
  uint64_t r15;
  uint64_t r14;
  uint64_t r13;
  uint64_t r12;
  uint64_t rbx;
  uint64_t rbp;
};

/* The thread's state */
enum State {
  /* Thread is available for use */
  Unused,
  /* Thread is being run */
  Running,
  /* Thread is currently suspended */
  Ready,
};

/* Describes a green thread, from the point of view of our lib. */
struct gt {
  /* Registers */
  struct gt_ctx ctx;

  /* State of operation */
  enum State state;
};

void gt_init(void);
void gt_ret(int ret);
bool gt_yield(void);
static void gt_stop(void);
extern void gt_switch(struct gt_ctx *old, struct gt_ctx *new);
int gt_go(void (*f)(void));

#endif
