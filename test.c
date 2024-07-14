#include "common.h"
#include "timer.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

#define N 1024
timer_task_t *tasks[N];
timer_mgr_t mgr = TIMER_MGR_INIT;

static void tasks_fill (void);
static void tasks_delete (void);
static void callback (void *arg);
static timer_task_t *task_new (int ms);

int
main (void)
{
  rand_init ();

  int tmfd, epfd;
  struct epoll_event ev;
  struct itimerspec its = {};

  if ((epfd = epoll_create1 (0)) == -1)
    abort ();

  if ((tmfd = timerfd_create (CLOCK_MONOTONIC, 0)) == -1)
    abort ();

  ev.data.fd = tmfd;
  ev.events = EPOLLIN;
  if (epoll_ctl (epfd, EPOLL_CTL_ADD, tmfd, &ev) == -1)
    abort ();

  tasks_fill ();
  tasks_delete ();

  for (int ms; (ms = timer_mgr_recent (&mgr)) != -1;)
    {
      its.it_value.tv_nsec = (ms % 1000) * 1000000;
      its.it_value.tv_sec = ms / 1000;

      if (timerfd_settime (tmfd, 0, &its, NULL) == -1)
        abort ();

      if (epoll_wait (epfd, &ev, 1, -1) == -1)
        abort ();

      if (ev.data.fd == tmfd)
        timer_mgr_exec (&mgr);
    }

  timer_mgr_free (&mgr);
  close (tmfd);
  close (epfd);
}

static void
tasks_delete (void)
{
  for (int i = 0; i < N * 4; i++)
    {
      timer_task_t *task;
      unsigned idx = rand_long (0, N);

      if ((task = tasks[idx]))
        {
          timer_mgr_del (&mgr, task);
          free (tasks[idx]);
          tasks[idx] = NULL;
        }
    }
}

static void
tasks_fill (void)
{
  for (int i = 0; i < N; i++)
    {
      int ms = rand_long (100, 2000);
      if (!timer_mgr_add (&mgr, (tasks[i] = task_new (ms))))
        abort ();
    }
}

static void
callback (void *arg)
{
  timer_task_t *task = arg;
  printf ("expire: %lu\n", task->expire);
  free (task);
}

static timer_task_t *
task_new (int ms)
{
  timer_task_t *new;

  if ((new = malloc (sizeof (timer_task_t))))
    {
      new->cb = callback;
      new->arg = new;

      new->expire = ms;
      new->times = 1;

      return new;
    }

  abort ();
}
