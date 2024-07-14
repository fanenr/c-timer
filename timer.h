#ifndef TIMER_H
#define TIMER_H

#include <stddef.h>
#include <time.h>

typedef void timer_cb_t (void *arg);
typedef struct timer_mgr_t timer_mgr_t;
typedef struct timer_task_t timer_task_t;

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
  unsigned times;
};

#define TIMER_MGR_INIT                                                        \
  (timer_mgr_t) {}

void timer_mgr_free (timer_mgr_t *mgr);

void timer_mgr_exec (timer_mgr_t *mgr);

int timer_mgr_recent (timer_mgr_t *mgr);

void timer_mgr_del (timer_mgr_t *mgr, timer_task_t *task);

timer_task_t *timer_mgr_add (timer_mgr_t *mgr, timer_task_t *task);

#endif
