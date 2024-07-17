#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>
#include <stddef.h>

typedef void timer_cb_t (void *arg);
typedef struct timer_mgr_t timer_mgr_t;
typedef struct timer_task_t timer_task_t;

#define attr_nonnull(...) __attribute__ ((nonnull (__VA_ARGS__)))

struct timer_mgr_t
{
  unsigned cap;
  unsigned size;
  timer_task_t **tasks;
};

struct timer_task_t
{
  void *arg;
  size_t expire;
  timer_cb_t *cb;
  unsigned index;
};

#define TIMER_MGR_INIT                                                        \
  (timer_mgr_t) {}

extern void timer_mgr_free (timer_mgr_t *mgr) attr_nonnull (1);

extern void timer_mgr_exec (timer_mgr_t *mgr) attr_nonnull (1);

extern int timer_mgr_recent (timer_mgr_t *mgr) attr_nonnull (1);

extern void timer_mgr_del (timer_mgr_t *mgr, timer_task_t *task)
    attr_nonnull (1, 2);

extern bool timer_mgr_add (timer_mgr_t *mgr, timer_task_t *task)
    attr_nonnull (1, 2);

#endif
