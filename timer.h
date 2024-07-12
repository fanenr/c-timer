#ifndef TIMER_H
#define TIMER_H

#include <stddef.h>
#include <time.h>

typedef void timer_cb_t (void *arg);
typedef struct timer_mgr_t timer_mgr_t;
typedef struct timer_task_t timer_task_t;

struct timer_mgr_t
{
  size_t cap;
  size_t size;
  timer_task_t *tasks;
};

struct timer_task_t
{
  void *arg;
  long expire;
  timer_cb_t *cb;
};

#define TIMER_MGR_INIT                                                        \
  (timer_mgr_t) {}

void timer_free (timer_mgr_t *mgr);

void timer_exec (timer_mgr_t *mgr);

int timer_recent (timer_mgr_t *mgr);

timer_task_t *timer_add (timer_mgr_t *mgr, int ms, timer_cb_t *cb, void *arg);

#endif
