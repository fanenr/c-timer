#include "timer.h"
#include <stdlib.h>

#define TIMER_INIT_CAP 8
#define TIMER_EXPAN_RATIO 2

#define LEFT(i) ((i) * 2 + 1)
#define RIGHT(i) ((i) * 2 + 2)
#define PARENT(i) (((i) - 1) / 2)

static long time_now (void);
static void timer_pop (timer_mgr_t *mgr);
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
  for (; mgr->size;)
    {
      timer_task_t *top = mgr->tasks;
      if (top->expire > time_now ())
        break;

      top->cb (top->arg);
      timer_pop (mgr);
    }
}

int
timer_recent (timer_mgr_t *mgr)
{
  if (!mgr->size)
    return -1;

  timer_task_t *top = mgr->tasks;
  long expire = top->expire;
  long now = time_now ();

  return expire > now ? expire - now : 0;
}

timer_task_t *
timer_add (timer_mgr_t *mgr, int ms, timer_cb_t *cb, void *arg)
{
  long expire;
  size_t cap = mgr->cap;
  size_t size = mgr->size;
  timer_task_t *tasks = mgr->tasks;

  if (size >= cap)
    {
      cap = cap ? cap * TIMER_EXPAN_RATIO : TIMER_INIT_CAP;
      if (!(tasks = realloc (tasks, cap * sizeof (timer_task_t))))
        return NULL;
      mgr->tasks = tasks;
      mgr->cap = cap;
    }

  if (ms <= 0 || (expire = time_now ()) == -1)
    return NULL;
  expire += ms;

  tasks[size] = (timer_task_t){
    .expire = expire,
    .arg = arg,
    .cb = cb,
  };

  return shift_up (tasks, mgr->size++);
}

#define swap(tasks, i, p)                                                     \
  do                                                                          \
    {                                                                         \
      timer_task_t temp = tasks[i];                                           \
      tasks[i] = tasks[p];                                                    \
      tasks[p] = temp;                                                        \
    }                                                                         \
  while (0)

static inline long
time_now (void)
{
  struct timespec ts;
  if (clock_gettime (CLOCK_MONOTONIC_COARSE, &ts) == 0)
    return ts.tv_sec * 1000 + ts.tv_nsec / (1000 * 1000);
  return -1;
}

static inline void
timer_pop (timer_mgr_t *mgr)
{
  size_t size = mgr->size;
  timer_task_t *tasks = mgr->tasks;

  if (!size)
    return;

  if (--size)
    {
      swap (tasks, 0, size);
      shift_down (tasks, size);
    }

  mgr->size--;
}

static inline timer_task_t *
shift_up (timer_task_t *tasks, size_t i)
{
  for (size_t p; i && (p = PARENT (i), tasks[i].expire < tasks[p].expire);
       i = p)
    swap (tasks, i, p);
  return tasks + i;
}

static inline timer_task_t *
shift_down (timer_task_t *tasks, size_t s)
{
  for (size_t i = 0, m, l, r;
       (m = i, l = LEFT (i), r = RIGHT (i),
       m = l < s && tasks[m].expire > tasks[l].expire ? l : m,
       m = r < s && tasks[m].expire > tasks[r].expire ? r : m, m != i);
       i = m)
    swap (tasks, m, i);
  return tasks;
}
