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
  int epfd;
  struct epoll_event evs[1];
  timer_mgr_t mgr = TIMER_MGR_INIT;

  for (size_t ms = 500; ms <= 4000; ms += ms)
    if (!timer_add (&mgr, ms, callback, NULL))
      abort ();

  if ((epfd = epoll_create1 (0)) == -1)
    abort ();

  for (int ms; (ms = timer_recent (&mgr)) != -1;)
    {
      epoll_wait (epfd, evs, 1, ms);
      timer_exec (&mgr);
    }

  timer_free (&mgr);
  close (epfd);
}
