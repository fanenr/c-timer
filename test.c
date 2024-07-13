#include "timer.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

void
callback (void *arg)
{
  (void)arg;
  printf ("callback\n");
}

int
main (void)
{
  int tmfd, epfd;
  struct epoll_event ev;
  struct itimerspec its = {};
  timer_mgr_t mgr = TIMER_MGR_INIT;

  if ((epfd = epoll_create1 (0)) == -1)
    abort ();

  if ((tmfd = timerfd_create (CLOCK_MONOTONIC, 0)) == -1)
    abort ();

  ev.data.fd = tmfd;
  ev.events = EPOLLIN;
  if (epoll_ctl (epfd, EPOLL_CTL_ADD, tmfd, &ev) == -1)
    abort ();

  for (size_t ms = 500; ms <= 4000; ms += ms)
    if (!timer_add (&mgr, ms, callback, NULL))
      abort ();

  for (int ms; (ms = timer_recent (&mgr)) != -1;)
    {
      its.it_value.tv_nsec = (ms % 1000) * 1000000;
      its.it_value.tv_sec = ms / 1000;

      if (timerfd_settime (tmfd, 0, &its, NULL) == -1)
        abort ();

      if (epoll_wait (epfd, &ev, 1, -1) == -1)
        abort ();

      if (ev.data.fd == tmfd)
        timer_exec (&mgr);
    }

  timer_free (&mgr);
  close (tmfd);
  close (epfd);
}
