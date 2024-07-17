#include "timer.h"

#include <stdlib.h>
#include <time.h>

#define LEFT(i) ((i) * 2 + 1)
#define RIGHT(i) ((i) * 2 + 2)
#define PARENT(i) (((i) - 1) / 2)

static size_t time_now (void);

#define mgr_top(mgr) (mgr->size ? mgr->tasks[0] : NULL)
static void mgr_pop (timer_mgr_t *mgr) attr_nonnull (1);
static bool mgr_expand (timer_mgr_t *mgr) attr_nonnull (1);

static void shift_up (timer_mgr_t *mgr, unsigned i) attr_nonnull (1);
static void shift_down (timer_mgr_t *mgr, unsigned i) attr_nonnull (1);

void
timer_mgr_free (timer_mgr_t *mgr)
{
  free (mgr->tasks);
  *mgr = TIMER_MGR_INIT;
}

void
timer_mgr_exec (timer_mgr_t *mgr)
{
  for (timer_task_t *top; (top = mgr_top (mgr)) && top->expire <= time_now ();
       mgr_pop (mgr))
    top->cb (top->arg);
}

int
timer_mgr_recent (timer_mgr_t *mgr)
{
  timer_task_t *top;
  size_t expire, now;

  if ((top = mgr_top (mgr)) && (now = time_now ()))
    return (expire = top->expire) > now ? expire - now : 0;

  return -1;
}

#define less(tasks, a, b) (tasks[a]->expire < tasks[b]->expire)
#define greater(tasks, a, b) (tasks[a]->expire > tasks[b]->expire)

void
timer_mgr_del (timer_mgr_t *mgr, timer_task_t *task)
{
  if (--mgr->size)
    {
      unsigned i = task->index, p = PARENT (i);
      mgr->tasks[i] = mgr->tasks[mgr->size];
      mgr->tasks[i]->index = i;

      if (!i || less (mgr->tasks, p, i))
        shift_down (mgr, i);
      else
        shift_up (mgr, i);
    }
}

bool
timer_mgr_add (timer_mgr_t *mgr, timer_task_t *task)
{
  size_t size = mgr->size, now;

  if (mgr_expand (mgr) && (now = time_now ()))
    {
      task->index = size;
      task->expire += now;

      mgr->tasks[size] = task;
      shift_up (mgr, mgr->size++);

      return true;
    }

  return false;
}

static inline size_t
time_now (void)
{
  struct timespec ts;

  if (clock_gettime (CLOCK_MONOTONIC_COARSE, &ts) == 0)
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

  return 0;
}

static inline void
mgr_pop (timer_mgr_t *mgr)
{
  if (--mgr->size)
    {
      mgr->tasks[0] = mgr->tasks[mgr->size];
      mgr->tasks[0]->index = 0;
      shift_down (mgr, 0);
    }
}

#define TIMER_INIT_CAP 8
#define TIMER_EXPAN_RATIO 2

static inline bool
mgr_expand (timer_mgr_t *mgr)
{
  unsigned cap = mgr->cap;
  timer_task_t **tasks = mgr->tasks;

  if (mgr->size < cap)
    return true;

  cap = cap ? cap * TIMER_EXPAN_RATIO : TIMER_INIT_CAP;
  if ((tasks = realloc (tasks, cap * sizeof (timer_task_t *))))
    {
      mgr->tasks = tasks;
      mgr->cap = cap;
      return true;
    }

  return false;
}

#define swap(tasks, a, b)                                                     \
  do                                                                          \
    {                                                                         \
      timer_task_t *temp = tasks[a];                                          \
      tasks[a]->index = b;                                                    \
      tasks[b]->index = a;                                                    \
      tasks[a] = tasks[b];                                                    \
      tasks[b] = temp;                                                        \
    }                                                                         \
  while (0)

static inline void
shift_up (timer_mgr_t *mgr, unsigned i)
{
  timer_task_t **tasks = mgr->tasks;
  for (unsigned p; i && (p = PARENT (i), less (tasks, i, p)); i = p)
    swap (tasks, i, p);
}

static inline void
shift_down (timer_mgr_t *mgr, unsigned i)
{
  unsigned s = mgr->size;
  timer_task_t **tasks = mgr->tasks;
  for (unsigned m, l, r; (m = i, l = LEFT (i), r = RIGHT (i),
                         m = l < s && greater (tasks, m, l) ? l : m,
                         m = r < s && greater (tasks, m, r) ? r : m, m != i);
       i = m)
    swap (tasks, i, m);
}
