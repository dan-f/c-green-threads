#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "gthr.h"

/* Proof-of-concept green thread implementation.

   - Follows https://c9x.me/articles/gthreads/code0.html
   - Assumes C99
   - Doesn't make use of OS threads; not OS thread safe */


/* Configures a maximum of 4 concurrent green threads, each with a stack size of
   4mb */
enum {
  MaxGreenThreads = 4,
  StackSize = 0x400000,
};

/* Global registry of green threads */
static struct gt gt_tbl[MaxGreenThreads];

/* Global pointer to current green thread */
static struct gt *gt_cur;

/* Global pointer to the "main" green thread */
static struct gt* gt_main = &gt_tbl[0];


/* Entry point for bootstrapping the lib.  Must be called before any other
   function.  Allocates the first green thread and marks it as running. */
void gt_init(void) {
  gt_cur = gt_main;
  gt_cur->state = Running;
}

/* Stops the current thread. */
void __attribute__((noreturn))
gt_ret(int ret) {
  /* Calling gt_ret on a non-main thread marks it as unused, and yields back for
     another thread to run. */
  if (gt_cur != gt_main) {
    gt_cur->state = Unused;
    gt_yield();
    assert(!"reachable");
  }
  /* Calling gt_ret on the main thread simply yields.
   * When there are no other threads left, we exit. */
  while (gt_yield())
    ;
  exit(ret);
}

/* Cooperatively yields control to another green thread. */
bool gt_yield(void) {
  struct gt *p;
  struct gt_ctx *old, *new;

  /* Scan for a thread to switch to.  If there are no ready threads, return
     false. */
  p = gt_cur;
  while (p->state != Ready) {
    if (++p == &gt_tbl[MaxGreenThreads])
      p = &gt_tbl[0];
    if (p == gt_cur)
      return false;
  }

  if (gt_cur->state != Unused)
    gt_cur->state = Ready;
  p->state = Running;
  old = &gt_cur->ctx;
  new = &p->ctx;
  gt_cur = p;
  // At this point, another thread picks up execution.
  // The context switch will save the current thread state, which is currently
  // in this function.
  gt_switch(old, new);
  // When some other gt_yield picks this thread to switch back to, we'll resume
  // from this point, returning `true` back to the user thread code.
  return true;
}

/* Helper for stopping a thread. */
static void gt_stop(void) {
  gt_ret(0);
}

/* Creates a new green thread. */
int gt_go(void (*f)(void)) {
  char *stack;                  /* The userland stack the thread will use */
  struct gt *p;                 /* The new green thread */

  for (p = &gt_tbl[0]; p->state != Unused; p++) {
    if (p == &gt_tbl[MaxGreenThreads]) {
      printf("DEBUG gt_go: failed to find an unused green thread.");
      /* Couldn't find an unused green thread */
      return -1;
    }
  }

  stack = malloc(StackSize);
  if (!stack) {
    printf("DEBUG gt_go: failed to malloc mem for the thread.");
    /* Couldn't malloc */
    return -1;
  }

  /* Put the address of gt_stop at the bottom of the stack */
  *(uint64_t *)&stack[StackSize - 8] = (uint64_t)gt_stop;
  /* Put the address of f above gt_stop */
  *(uint64_t *)&stack[StackSize - 16] = (uint64_t)f;
  /* Set the stack pointer to where f is */
  p->ctx.rsp = (uint64_t)&stack[StackSize - 16];
  p->state = Ready;

  return 0;
}
