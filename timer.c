#include "timer.h"

#include <stdbool.h>
#include <stdlib.h>

#define LEFT(i) ((i) * 2 + 1)
#define RIGHT(i) ((i) * 2 + 2)
#define PARENT(i) (((i) - 1) / 2)

static size_t time_now (void);

static void timer_pop (timer_mgr_t *mgr);
static bool timer_expand (timer_mgr_t *mgr);
#define timer_top(mgr) (mgr->size ? mgr->tasks : NULL)

static timer_task_t *shift_up (timer_task_t *tasks, size_t i);
static timer_task_t *shift_down (timer_task_t *tasks, size_t s);

void
timer_free (timer_mgr_t *mgr)
{
  free (mgr->tasks);
  *mgr = TIMER_MGR_INIT;
}

void
timer_exec (timer_mgr_t *mgr)
{
  for (timer_task_t *top;
       (top = timer_top (mgr)) && top->expire <= time_now (); timer_pop (mgr))
    top->cb (top->arg);
}

int
timer_recent (timer_mgr_t *mgr)
{
  timer_task_t *top;
  size_t expire, now;

  if ((top = timer_top (mgr)) && (now = time_now ()))
    return (expire = top->expire) > now ? expire - now : 0;

  return -1;
}

timer_task_t *
timer_add (timer_mgr_t *mgr, int ms, timer_cb_t *cb, void *arg)
{
  size_t expire;

  if (!timer_expand (mgr))
    return NULL;

  if (ms <= 0 || !(expire = time_now ()))
    return NULL;
  expire += ms;

  mgr->tasks[mgr->size] = (timer_task_t){
    .expire = expire,
    .arg = arg,
    .cb = cb,
  };

  return shift_up (mgr->tasks, mgr->size++);
}

#define swap(tasks, a, b)                                                     \
  do                                                                          \
    {                                                                         \
      timer_task_t temp = tasks[a];                                           \
      tasks[a] = tasks[b];                                                    \
      tasks[b] = temp;                                                        \
    }                                                                         \
  while (0)

static inline size_t
time_now (void)
{
  struct timespec ts;

  if (clock_gettime (CLOCK_MONOTONIC_COARSE, &ts) == 0)
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

  return 0;
}

static inline void
timer_pop (timer_mgr_t *mgr)
{
  if (mgr->size && --mgr->size)
    {
      swap (mgr->tasks, 0, mgr->size);
      shift_down (mgr->tasks, mgr->size);
    }
}

#define TIMER_INIT_CAP 8
#define TIMER_EXPAN_RATIO 2

static inline bool
timer_expand (timer_mgr_t *mgr)
{
  size_t cap = mgr->cap;
  timer_task_t *tasks = mgr->tasks;

  if (mgr->size < cap)
    return true;

  cap = cap ? cap * TIMER_EXPAN_RATIO : TIMER_INIT_CAP;
  if ((tasks = realloc (tasks, cap * sizeof (timer_task_t))))
    {
      mgr->tasks = tasks;
      mgr->cap = cap;
      return true;
    }

  return false;
}

#define less(tasks, a, b) (tasks[a].expire < tasks[b].expire)
#define greater(tasks, a, b) (tasks[a].expire > tasks[b].expire)

static inline timer_task_t *
shift_up (timer_task_t *tasks, size_t i)
{
  for (unsigned p; i && (p = PARENT (i), less (tasks, i, p)); i = p)
    swap (tasks, i, p);
  return tasks + i;
}

static inline timer_task_t *
shift_down (timer_task_t *tasks, size_t s)
{
  for (unsigned i = 0, m, l, r;
       (m = i, l = LEFT (i), r = RIGHT (i),
       m = l < s && greater (tasks, m, l) ? l : m,
       m = r < s && greater (tasks, m, r) ? r : m, m != i);
       i = m)
    swap (tasks, i, m);
  return tasks;
}
