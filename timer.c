#include "timer.h"
#include <stdlib.h>

#define TIMER_INIT_CAP 8
#define TIMER_EXPAN_RATIO 2

#define LEFT(i) ((i) * 2 + 1)
#define RIGHT(i) ((i) * 2 + 2)
#define PARENT(i) (((i) - 1) / 2)

static long time_now (void);

static void timer_pop (timer_mgr_t *mgr);
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
  for (timer_task_t *top; (top = timer_top (mgr));)
    {
      if (top->expire > time_now ())
        break;

      top->cb (top->arg);
      timer_pop (mgr);
    }
}

int
timer_recent (timer_mgr_t *mgr)
{
  timer_task_t *top;

  if ((top = timer_top (mgr)))
    {
      long now = time_now ();
      long expire = top->expire;
      return expire > now ? expire - now : 0;
    }

  return -1;
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

#define swap(tasks, a, b)                                                     \
  do                                                                          \
    {                                                                         \
      timer_task_t temp = tasks[a];                                           \
      tasks[a] = tasks[b];                                                    \
      tasks[b] = temp;                                                        \
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

  if (size && --size)
    {
      swap (tasks, 0, size);
      shift_down (tasks, size);
    }

  --mgr->size;
}

#define less(tasks, a, b) (tasks[a].expire < tasks[b].expire)
#define greater(tasks, a, b) (tasks[a].expire > tasks[b].expire)

static inline timer_task_t *
shift_up (timer_task_t *tasks, size_t i)
{
  for (size_t p; i && (p = PARENT (i), less (tasks, i, p)); i = p)
    swap (tasks, i, p);
  return tasks + i;
}

static inline timer_task_t *
shift_down (timer_task_t *tasks, size_t s)
{
  for (size_t i = 0, m, l, r;
       (m = i, l = LEFT (i), r = RIGHT (i),
       m = l < s && greater (tasks, m, l) ? l : m,
       m = r < s && greater (tasks, m, r) ? r : m, m != i);
       i = m)
    swap (tasks, i, m);
  return tasks;
}
